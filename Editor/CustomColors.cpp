#include "Precompile.h"
#include "CustomColors.h"

#include "Platform/Console.h"
#include "Foundation/Tokenize.h"

#include <sstream>

using namespace Helium;

static const std::string s_ColorDelimiter = "#";

///////////////////////////////////////////////////////////////////////////////
// Gets the registry key for where the custom colors should be stored by default.
// 
std::string CustomColors::GetConfigKey()
{
    return "CustomColors";
}

///////////////////////////////////////////////////////////////////////////////
// Iterates over all the custom colors in colorData and builds a delimited
// string representation of the information.  This information can be persisted
// by the application and reloaded later by calling Load.
// 
std::string CustomColors::Save( wxColourData& colorData )
{
    std::ostringstream result;

    for ( int32_t colorIndex = 0; colorIndex < NumColors; ++colorIndex )
    {
        if ( colorIndex > 0 )
        {
            result << s_ColorDelimiter;
        }

        const wxColour& color = colorData.GetCustomColour( colorIndex );
        if ( color.IsOk() )
        {
            // GetAsString returns HTML format (example: #FF00FF).  Strip the leading #.
            result << static_cast< const char* >( color.GetAsString( wxC2S_HTML_SYNTAX ).substr( 1 ).c_str() );
        }
    }

    return result.str();
}

///////////////////////////////////////////////////////////////////////////////
// Populates the custom colors of colorData with the values stored in the info
// string.  The info string is expected to have been generated by calling the
// above Save function.
// 
void CustomColors::Load( wxColourData& colorData, const std::string& info )
{
    if ( !info.empty() )
    {
        std::vector< std::string > colors;
        Tokenize( info, colors, s_ColorDelimiter );

        std::vector< std::string >::const_iterator colorItr = colors.begin();
        std::vector< std::string >::const_iterator colorEnd = colors.end();
        for ( int32_t colorIndex = 0; colorItr != colorEnd && colorIndex < NumColors; ++colorItr, ++colorIndex )
        {
            const std::string& colorStr = *colorItr;
            if ( colorStr.length() >= 6 )
            {
                uint32_t red = 0;
                uint32_t green = 0;
                uint32_t blue = 0;
                StringScan( colorStr.c_str(), "%02X%02X%02X", &red, &green, &blue );
                colorData.SetCustomColour( colorIndex, wxColour( (uint8_t)red, (uint8_t)green, (uint8_t)blue ) );
            }
        }
    }
}
