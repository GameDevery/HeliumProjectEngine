#include "Precompile.h"
#include "P4Provider.h"
#include "Perforce.h"
#include "P4Exceptions.h"
#include "P4QueryCommands.h"
#include "P4ClientCommands.h"
#include "P4ChangelistCommands.h"

#include "Platform/Assert.h"
#include "Platform/Locks.h"

#include "Application/Message.h"
#include "Application/RCS.h"
#include "Application/Startup.h"

using namespace Helium;
using namespace Helium::Perforce;

WaitSignature::Delegate Perforce::g_ShowWaitDialog;
Perforce::MessageSignature::Delegate Perforce::g_ShowWarningDialog;
Profile::Sink g_CommandSink( "Perforce Commands" );

#ifdef _DEBUG
//#define PERFORCE_DEBUG_CONNECT
#endif

Provider::Provider()
	: m_IsEnabled( true )
	, m_IsInitialized( false )
	, m_IsConnected( false )
	, m_Abort( false )
	, m_Shutdown( false )
	, m_Execute( true, false )
	, m_Completed( true, false )
	, m_Command( NULL )
	, m_Phase( CommandPhases::Unknown )
{
	if ( IsDebuggerPresent() )
	{
		m_ConnectionTestTimeout = 0xffffffff;
		m_ForegroundExecuteTimeout = 0xffffffff;
		m_BackgroundExecuteTimeout = 0xffffffff;
	}
	else
	{
		m_ConnectionTestTimeout = 100;
		m_ForegroundExecuteTimeout = 5000;
		m_BackgroundExecuteTimeout = 10000;
	}
}

Provider::~Provider()
{
	Cleanup();
}

void Provider::Initialize()
{
	m_IsInitialized = true;

	Helium::CallbackThread::Entry entry = &Helium::CallbackThread::EntryHelper<Provider, &Provider::ThreadEntry>;
	if ( !m_Thread.Create( entry, this, "Perforce Transaction Thread" ) )
	{
		throw Perforce::Exception( "Unable to create thread for perforce transaction" );
	}
}

void Provider::Cleanup()
{
	if ( m_IsConnected )
	{
		if ( !m_Client.Dropped() )
		{
			Error e;
			m_Client.Final( &e );
		}
	}

	if ( m_IsInitialized )
	{
		m_Shutdown = true;
		m_Execute.Signal();
		m_Thread.Join();
	}
}

void Provider::ThreadEntry()
{
	while ( !m_Shutdown )
	{
		m_Execute.Wait();
		m_Execute.Reset();

		if ( m_Shutdown )
		{
			break;
		}

		if ( m_Command )
		{
			std::string cmd;
			bool converted = Helium::ConvertString( m_Command->m_Command, cmd );
			HELIUM_ASSERT( converted );

			std::vector< const char* > args;

			std::vector< std::string >::const_iterator itr = m_Command->m_Arguments.begin();
			std::vector< std::string >::const_iterator end = m_Command->m_Arguments.end();
			for ( ; itr != end; ++itr )
			{
				cmd += " " + *itr;
				args.push_back( (*itr).c_str() );
			}

			if ( m_Command->m_Arguments.size() > 0 )
			{
				m_Client.SetArgv( (int)args.size(), const_cast<char**>( &args.front() ) );
			}

			Helium::Profile::Timer scopeTimer ( g_CommandSink, "Command 'p4 %s'", cmd.c_str() );
			Log::Debug( "%s\n", cmd.c_str() );
			m_Client.Run( cmd.c_str(), m_Command );
			m_Phase = CommandPhases::Complete;
			m_Completed.Signal();
		}
	}
}

void Provider::RunCommand( Command* command )
{
	if ( !m_IsEnabled )
	{
		throw Perforce::Exception( "Perforce connection is not enabled" );
	}

	Helium::MutexScopeLock mutex ( m_Mutex );

	m_Abort = false;
	m_Phase = CommandPhases::Unknown;

	//
	// This do/while loop is necessary because...
	//  - if we fail to execute the command we can re-connect (or wait and reconnect), and try the command again:
	//     trying the command again will happen frequently during dropped connection situations, because we cannot
	//     test the dropped status except for immediately after attempting to run a command, so in the case of a 
	//     dropped connection, we will always fail the first try, then re-connect and succeed
	//
	// Watch our for Connect(), don't call it in the main line scenarios
	//  - the most frequent case through this code is while we are already connected, so we don't always call Connect()
	//     because its very expensive... we only bother to call Connect() after we try to run a command and fail this is
	//     because testing the connection with confidence requires sending some sort of command, whether that be info
	//     or the command that the calling api wants executed...
	//

	do
	{
		if ( m_IsConnected )
		{
			m_Phase = CommandPhases::Executing;

			m_Completed.Reset();
			m_Command = command;
			m_Execute.Signal();

			if ( Thread::IsMain() )
			{
				if ( !m_Completed.Wait( m_ForegroundExecuteTimeout ) )
				{
					if ( g_ShowWaitDialog.Valid() )
					{
						WaitArgs args ( this );
						g_ShowWaitDialog.Invoke( args );
						if ( args.m_Cancel )
						{
							m_IsEnabled = false;
							m_Abort = true;
						}
					}
				}
			}
			else
			{
				if ( !m_Completed.Wait( m_BackgroundExecuteTimeout ) )
				{
					m_Abort = true;
				}
			}

			// this will wait for the transaction to complete (flagged for abort if the user cancelled it), or fall through if its done
			m_Completed.Wait();
		}

		if ( m_Client.Dropped() )
		{
			m_IsConnected = false;
		}

		if ( !m_IsConnected )
		{
			m_ConnectTimer.Reset();

			m_Phase = CommandPhases::Connecting;

			if ( !Connect() )
			{
				if ( Thread::IsMain() )
				{
					// this will poll Connect() in a timer
					if ( g_ShowWaitDialog.Valid() )
					{
						WaitArgs args ( this );
						g_ShowWaitDialog.Invoke( args );
						if ( args.m_Cancel )
						{
							m_IsEnabled = false;

							if ( g_ShowWarningDialog.Valid() )
							{
								g_ShowWarningDialog.Invoke( MessageArgs( "Warning: Continuing to work without a perforce connection could expose unexpected problems.\n\nPlease consider saving your work and waiting until the connection can be restored.", "Warning" ) );
							}

							throw Perforce::Exception( "Failed to connect to perforce server" );
						}
					}
				}

				if ( m_IsConnected )
				{
					Log::Print( "Connection to Perforce has been established\n" );
				}
			}
		}
	}
	while ( m_Phase != CommandPhases::Complete && m_IsEnabled && m_IsConnected && !m_Abort && command->m_ErrorCount == 0 );

	if ( Thread::IsMain() )
	{
		if ( g_ShowWaitDialog.Valid() )
		{
			// this will cancel the UI dialog if the transaction completed (and was not aborted)
			m_Abort = true;
		}
	}
	else
	{
		if ( m_Abort )
		{
			// we timed out in a background thread, throw
			throw Perforce::Exception( "Perforce transaction timed out (timeout = %d)", m_BackgroundExecuteTimeout);
		}
	}

	if ( command->m_ErrorCount )
	{
		HELIUM_ASSERT_MSG( !command->m_ErrorString.empty(), "No error string was captured from a failed perforce command, this indicates a command object is not properly interpreting the server's output" );
		throw Perforce::Exception( "%d error%s for command '%s':\n%s", command->m_ErrorCount, command->m_ErrorCount > 1 ? "s" : "", command->AsString().c_str(), command->m_ErrorString.c_str() );
	}
}

bool Provider::Connect()
{
	if ( m_IsConnected )
	{
		// This extra 'info' command is unfortunate but necessary
		//  .Dropped() can only be trusted immediately after .Run(), so do a lightweight run here to update .Dropped()'s state

		class NullClient : public ClientUser
		{
		public:
			virtual void OutputInfo( char level, const char* data ) {}
			virtual void OutputError( const char* data )            {}
		} nullClient;

		m_Client.SetBreak( this );
		m_Client.Run( "info", &nullClient );
	}

	if ( m_Client.Dropped() )
	{
		Error e;
		m_Client.Final( &e );
		m_IsConnected = false;

#ifdef PERFORCE_DEBUG_CONNECT
		if ( e.Test() )
		{
			StrBuf buf;
			e.Fmt( &buf, EF_PLAIN );
			Log::Warning( "%s\n", buf.Text() );
		}
#endif
	}

	if ( !m_IsConnected )
	{
		Error e;
		m_Client.SetProtocol( "tag", "" );
		m_Client.Init( &e );
		m_Client.SetBreak( this );

#ifdef PERFORCE_DEBUG_CONNECT
		if ( e.Test() )
		{
			StrBuf buf;
			e.Fmt( &buf, EF_PLAIN );
			Log::Warning( "%s\n", buf.Text() );
		}
#endif

		char buf[ 64 ];
		StringPrint( buf, sizeof(buf), "HeliumEditor" );
		buf[ sizeof(buf) - 1 ] = 0; 
		m_Client.SetProg( buf );

		bool converted = Helium::ConvertString( m_Client.GetUser().Text(), m_UserName );
		HELIUM_ASSERT( converted );

		converted = Helium::ConvertString( m_Client.GetClient().Text(), m_ClientName );
		HELIUM_ASSERT( converted );

		m_IsConnected = e.Test() == 0;
	}

	return m_IsConnected;
}

// this instructs when a p4api transaction should keep waiting (return true to keep waiting)
int	Provider::IsAlive()
{
	switch ( m_Phase )
	{
	case CommandPhases::Connecting:
		{
			return m_ConnectTimer.Elapsed() <= m_ConnectionTestTimeout;
		}

	case CommandPhases::Executing:
		{
			return !m_Abort;
		}

	default:
		{
			// this would not make any sense... why would we be complete in the Alive callback?
			HELIUM_ASSERT( m_Phase != CommandPhases::Complete );
			return true;
		}
	}
}

// this instruct our waiting UI to stop waiting (return true to stop waiting)
bool Provider::StopWaiting()
{
	switch ( m_Phase )
	{
	case CommandPhases::Executing:
		{
			return m_Abort;
		}

	case CommandPhases::Connecting:
		{
			return Connect();
		}

	default:
		{
			// this is the natural closing of the dialog if the server comes back online and completes successfully
			HELIUM_ASSERT( m_Phase == CommandPhases::Complete );
			return true;
		}
	}
}

///////////////////////////////////////////////////////////////////
// Implementation

bool Provider::IsEnabled()
{
	return m_IsEnabled;
}

void Provider::SetEnabled( bool enabled )
{
	m_IsEnabled = enabled;
}

const char* Provider::GetName()
{
	return "Perforce";
}

void Provider::Sync( RCS::File& file, const uint64_t timestamp )
{
	SyncCommand command( this, &file, timestamp );

	command.Run();
}

void Provider::GetInfo( RCS::File& file, const RCS::GetInfoFlag flags )
{
	SingleFStatCommand command( this, &file );
	command.Run();

	if ( ( flags & RCS::GetInfoFlags::GetHistory ) == RCS::GetInfoFlags::GetHistory )
	{
		FileLogCommand filelogCommand( this, &file, ( ( flags & RCS::GetInfoFlags::GetIntegrationHistory ) == RCS::GetInfoFlags::GetIntegrationHistory ) );
		filelogCommand.Run();
	}
}

void Provider::GetInfo( const std::string& folder, RCS::V_File& files, bool recursive, uint32_t fileData, uint32_t actionData )
{
	MultiFStatCommand command( this, folder, &files, recursive, fileData, actionData );
	command.Run();
}

void Provider::Add( RCS::File& file )
{
	OpenCommand command( this, "add", &file );
	command.Run();
}

void Provider::Edit( RCS::File& file )
{
	OpenCommand command ( this, "edit", &file );
	command.Run();
}

void Provider::Delete( RCS::File& file )
{
	OpenCommand command ( this, "delete", &file );
	command.Run();
}

void Provider::Integrate( RCS::File& source, RCS::File& dest )
{
	IntegrateCommand command( this, &source, &dest );
	command.Run();
}

void Provider::Reopen( RCS::File& file )
{
	OpenCommand command( this, "reopen", &file );
	command.Run();
}

void Provider::GetChangesets( RCS::V_Changeset& changesets )
{
	GetChangelistsCommand command( this, &changesets );
	command.Run();
}

void Provider::CreateChangeset( RCS::Changeset& changeset )
{
	CreateChangelistCommand command( this, &changeset );
	command.Run();
}

void Provider::GetOpenedFiles( RCS::V_File& files )
{
	OpenedCommand command( this, &files );
	command.Run();
}

void Provider::Commit( RCS::Changeset& changeset )
{
	SubmitCommand command( this, &changeset );

	command.AddArg( "-c" );
	command.AddArg( RCS::GetChangesetIdAsString( changeset.m_Id ) );

	command.Run();
}

void Provider::Revert( RCS::Changeset& changeset, bool revertUnchangedOnly )
{
	RevertCommand command( this );

	command.AddArg( "-c" );
	command.AddArg( RCS::GetChangesetIdAsString( changeset.m_Id ) );

	if ( revertUnchangedOnly )
	{
		command.AddArg( "-a" );
	}

	command.AddArg( "//..." ); // careful

	command.Run();
}


void Provider::Revert( RCS::File& file, bool revertUnchangedOnly )
{
	RevertCommand command( this, &file );

	command.AddArg( "-c" );
	command.AddArg( RCS::GetChangesetIdAsString( file.m_ChangesetId ) );

	if ( revertUnchangedOnly )
	{
		command.AddArg( "-a" );
	}

	std::string narrowPath;
	bool converted = Helium::ConvertString( file.m_LocalPath, narrowPath );
	HELIUM_ASSERT( converted );

	command.AddArg( narrowPath.c_str() );

	command.Run();
}

void Provider::Rename( RCS::File& source, RCS::File& dest )
{
	IntegrateCommand integrateCommand( this, &source, &dest );
	integrateCommand.Run();

	OpenCommand deleteCommand( this, "delete", &source );
	deleteCommand.Run();
}
