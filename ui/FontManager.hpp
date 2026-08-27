#pragma once

#include <wx/font.h>
#include <wx/string.h>
#include <string>

class FontManager
{
public:

    static bool Initialize();

    static wxFont Regular(int size);
    static wxFont Medium(int size);
    static wxFont SemiBold(int size);
    static wxFont Bold(int size);

private:

    static bool m_Initialized;

    // Resolved at Initialize() time: "Inter" only if every weight's TTF
    // actually registered successfully, otherwise "Segoe UI" -- the
    // native modern Windows UI font -- so a failed/missing font file
    // never silently degrades into whatever ugly default face Windows
    // picks for an unresolved family name.
    static wxString m_ActiveFamily;

    static const std::string FontFolder;

    static const std::string RegularPath;
    static const std::string MediumPath;
    static const std::string SemiBoldPath;
    static const std::string BoldPath;
};
