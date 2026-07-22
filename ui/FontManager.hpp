#pragma once

#include <wx/font.h>
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

    static std::string m_FontFolder;

    static std::string RegularPath;
    static std::string MediumPath;
    static std::string SemiBoldPath;
    static std::string BoldPath;
};