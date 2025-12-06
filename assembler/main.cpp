#include <iostream>
#include <string>
#include <map>
#include <cstdint>
#include <fstream>
#include <vector>
#include <bitset>
#include <nlohmann/json.hpp>

std::map<std::string, uint8_t> opcodes = {
    {"MOV", 0},
    {"ADD", 1},
    {"SUB", 2},
    {"NEG", 3},
    {"MULTL", 4},
    {"MULTH", 5},
    {"ROTR", 6},
    {"RAND", 7},
    {"XOR", 8},
    {"OR", 9},
    {"AND", 10},
    {"CMP", 11},
    {"INPT", 12},
    {"SHFTR", 13},
    {"SHFTL", 14},
    {"JZ", 15},
    {"JZA", 15},
    {"SCN", 0},
    {"HALT", 15},
    {"DRAW", 16},
    {"CLEAR", 32}
};

std::map<std::string, std::string> mem_codes = {
    {"#", "00"},
    {"", "00"},
    {"A", "01"},
    {"B", "11"},
    {"$", "10"},
};

std::map<std::string, std::string> mem_codes1 = {
    {"A", "00"},
    {"B", "10"},
    {"$", "01"},
    {"%", "11"},
};

std::map<std::string, int> labels = {};
std::map<std::string, int> defines = {};

std::string reverse(const std::string bits)
{
    std::string reversed;
    for (int i = bits.length() - 1; i >= 0; i--)
    {
        reversed += bits[i];
    }
    return reversed;
}

std::vector<std::string> outR1;
std::vector<std::string> outR2;
std::vector<std::string> outR3;

void parse_labels(std::string str, int c)
{
    if (str.at(0) == '.')
    {
        labels.emplace(str.substr(0, str.find_first_of(' ')), c);
        std::cout << str.substr(0, str.find_first_of(' ')) << " : " << c << "\n";
    }
}

void parse_defines(std::string str, int c)
{
    if (str.substr(0, 7) == "#define")
    {
        str = str.substr(str.find_first_of(' ')+1);
        std::cout << str << "\n";
        std::string str1 = str.substr(0, str.find_first_of(' '));
        str = str.substr(str.find_first_of(' ')+1);
        std::cout << str << "\n";
        std::bitset<8> bits;
        try {
            if (str.substr(0, 2) == "0b") throw std::runtime_error("");
            bits = std::stoi(str.substr(0, str.find_first_of(' ')));
        } catch (std::exception& e1) {
            std::bitset<8> bitst(str.substr(0, str.find_first_of(' ')).substr(2));
            bits = bitst;
        }
        int str2 = bits.to_ulong();
        defines.emplace(str1, str2);
        std::cout << str1 << " = " << str2 << "\n";
    }
}

void parse(std::string str, int c)
{
    std::string rom1 = "00000000";
    std::string rom2 = "";
    std::string rom31 = "";
    std::string rom32 = "";

    bool dynamic = false;
    bool skip = false;
    bool screenR = false;
    int breakP = 0;

    if (str.at(0) == '.')
    {
        breakP = std::min(std::min(std::min(str.find_first_of(' '), str.find_first_of(',')), std::min(str.find_first_of('>') - 1, str.find_first_of('-') - 1)), str.length());
        str = str.substr(str.find_first_not_of(' ', breakP + 1), str.length());
    }

    std::cout << str << "\n";

    int closest = std::min(std::min(str.find_first_of(' '), str.find_first_of(',')), str.length());
    std::string arg = str.substr(0, closest);
    std::cout << arg << "\n";
    int opcode = opcodes[arg];
    std::bitset<8> op(opcode);
    rom1 += op.to_string();
    if (arg == "HALT")
    {
        std::bitset<8> bits(c);
        outR1.push_back(rom1);
        outR2.push_back("0000000000000000");
        outR3.push_back("00000000" + bits.to_string());
        return;
    }
    breakP = std::min(std::min(std::min(str.find_first_of(' '), str.find_first_of(',')), std::min(str.find_first_of('>') - 1, str.find_first_of('-') - 1)), str.length());
    str = str.substr(std::min(str.find_first_not_of(' ', breakP + 1), str.length()), str.length());
    std::cout << str << "\n";

    closest = std::min(std::min(std::min(str.find_first_of(' '), str.find_first_of(',')), str.length()), str.find_first_of('-'));
    std::string arg1 = str.substr(0, closest);
    if (str.substr(0, 2) == "->" || str.length() < 1 || arg1.at(0) == ';')
    {
        skip = true;
    }
    if (!skip)
    {
        if (arg1.at(0) == '.')
        {
            arg1 = std::to_string(labels[arg1]);
        }
        std::cout << arg1 << "\n";
        std::string code1 = mem_codes[arg1.substr(0, 1)];
        int checkP = 1;
        if (code1 == "")
        {
            code1 = "00";
            checkP = 0;
        }
        if (code1 == "10")
        {
            arg1 = arg1.substr(1, arg1.length());
        }
        if (arg1.substr(checkP, checkP+1) == "!") {
            screenR = true;
            arg1 = arg1.substr(1, arg1.length());
        }
        std::cout << code1 << "\n";
        rom2 += code1;
        if (arg1 != "A" && arg1 != "B")
        {
            std::bitset<8> bits1 = 0;
            
            try {
                if (arg1.substr(0, 2) == "0b") throw std::runtime_error("");
                if (arg == "JZA") {
                    bits1 = std::stoi(arg1) + c;
                } else {
                    bits1 = std::stoi(arg1);
                }
            } catch (std::exception& e) {
                try {
                    if (arg1.length() < 8) throw std::runtime_error("");
                    std::bitset<8> bitst(arg1.substr(2));
                    bits1 = bitst;
                } catch (std::exception& e1) {
                    bits1 = defines[arg1];
                }
            }
            rom31 += bits1.to_string();
        }
        else
        {
            rom31 += "00000000";
        }
        breakP = std::min(std::min(std::min(str.find_first_of(' '), str.find_first_of(',')), std::min(str.find_first_of('>') - 1, str.find_first_of('-') - 1)), str.length());
        str = str.substr(std::min(str.find_first_not_of(' ', breakP + 1), str.length()), str.length());
        std::cout << str << "\n";
    }
    else
    {
        rom2 += "00";
        rom31 += "00000000";
        std::cout << "skip" << "\n";
    }

    closest = std::min(std::min(std::min(str.find_first_of(' '), str.find_first_of(',')), str.length()), str.find_first_of('-'));
    std::string arg2 = str.substr(0, closest);
    if (str.substr(0, 2) == "->" || str.length() < 1 || arg2.at(0) == ';')
    {
        skip = true;
    }
    if (str.substr(0, 2) == "<-")
    {
        dynamic = true;
        skip = true;
    }
    if (!skip)
    {
        if (arg2.at(0) == '.')
        {
            arg2 = std::to_string(labels[arg2]);
        }
        std::cout << arg2 << "\n";
        std::string code2 = mem_codes[arg2.substr(0, 1)];
        if (code2 == "")
        {
            code2 = "00";
        }
        if (code2 == "10")
        {
            arg2 = arg2.substr(1, arg2.length());
        }
        std::cout << code2 << "\n";
        rom2 += code2;
        if (arg2 != "A" && arg2 != "B")
        {
            std::bitset<8> bits2 = 0;
            try {
                if (arg2.substr(0, 2) == "0b") throw std::runtime_error("");
                bits2 = std::stoi(arg2);
            } catch (std::exception& e) {
                try {
                    if (arg2.length() < 8) throw std::runtime_error("");
                    std::bitset<8> bitst(arg2.substr(2));
                    bits2 = bitst;
                } catch (std::exception& e1) {
                    bits2 = defines[arg2];
                }
            }
            rom32 += bits2.to_string();
        }
        else
        {
            rom32 += "00000000";
        }

        breakP = std::min(std::min(std::min(str.find_first_of(' '), str.find_first_of(',')), std::min(str.find_first_of('>') - 1, str.find_first_of('-') - 1)), str.length());
        str = str.substr(std::min(str.find_first_not_of(' ', breakP + 1), str.length()), str.length());
        std::cout << str << "\n";
    }
    else
    {
        breakP = std::min(std::min(str.find_first_of(' '), str.find_first_of(',')), std::max(str.find_first_of('>'), str.find_first_of('-')));
        std::string str1 = str.substr(std::min(str.find_first_not_of(' ', breakP + 1), str.length()), str.length());
        closest = std::min(std::min(std::min(str1.find_first_of(' '), str1.find_first_of(',')), str1.length()), str1.find_first_of('-'));
        arg2 = str1.substr(0, closest);
        std::cout << arg2 << "\n";
        std::string code2 = mem_codes[arg2.substr(0, 1)];
        if (code2 == "" || (code2.at(1) == '1' && arg2.length() > 1))
        {
            code2 = "00";
        }
        rom2 += code2;
        rom32 += "00000000";
        std::cout << code2 << "\n";
        std::cout << "skip" << "\n";
    }

    bool noOut = false;
    if (str.length() < 2)
    {
        noOut = true;
    }
    else
    {
        std::string arg3 = str.substr(0, 2);

        if (arg3 == "<-")
        {
            dynamic = true;
        }
        else if (arg3 != "->")
        {
            noOut = true;
        }
    }

    std::cout << "noOut: " << noOut << "  dynamic: " << dynamic << "\n";

    if (!noOut)
    {
        breakP = std::min(std::min(str.find_first_of(' '), str.find_first_of(',')), std::max(str.find_first_of('>'), str.find_first_of('-')));
        str = str.substr(std::min(str.find_first_not_of(' ', breakP + 1), str.length()), str.length());

        closest = std::min(std::min(str.find_first_of(' '), str.find_first_of(',')), str.length());
        std::string arg4 = str.substr(0, closest);

        std::cout << arg4 << "\n";

        std::string code4 = "00";

        if (!dynamic)
        {
            code4 = mem_codes1[arg4.substr(0, 1)];
            std::cout << code4 << "\n";
        } else {
            code4 = "11";
        }

        rom2 += code4;

        if (screenR)
        {
            rom2 += "1";
        }
        else
        {
            rom2 += "0";
        }

        if (dynamic)
        {
            rom2 += "1";
        }
        else
        {
            rom2 += "0";
        }

        if (arg4 != "A" && arg4 != "B")
        {
            std::bitset<8> bits3(0);
            if (code4 == "00" || (arg == "MOV" && dynamic))
            {
                try {
                    bits3 = std::stoi(arg4.substr(0, arg4.length()));
                } catch (std::exception& e) {
                    bits3 = defines[arg4.substr(0, arg4.length())];
                }
            }
            else
            {
                try {
                    bits3 = std::stoi(arg4.substr(1, arg4.length()));
                } catch (std::exception& e) {
                    bits3 = defines[arg4.substr(1, arg4.length())];
                }
            }
            std::cout << bits3 << "\n";
            rom2 += reverse(bits3.to_string());
        }
        else
        {
            rom2 += "00000000";
        }
    }
    else
    {
        rom2 += "000000000000";
    }

    std::cout << "\n\n";

    rom2 = reverse(rom2);

    outR1.push_back(rom1);
    outR2.push_back(rom2);
    outR3.push_back(rom32 + rom31);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "No file to compile!";
        return 0;
    }
    std::ifstream input_file("C:\\Users\\ASUS\\AppData\\LocalLow\\SebastianLague\\Digital-Logic-Sim\\Projects\\MUCH_BETTER_CPU\\Chips\\CPU.json");
    nlohmann::json data = nlohmann::json::parse(input_file);
    input_file.close();
    std::ifstream code(argv[1]);
    std::ifstream code1(argv[1]);
    std::ifstream code2(argv[1]);
    std::string line;
    int i = 0;
    try
    {
        while (std::getline(code1, line))
        {
            if (line.length() < 2)
                continue;
            line = line.substr(line.find_first_not_of(' '), line.length());
            if (line.at(0) != ';' && line.at(0) != '#')
            {
                parse_labels(line, i++);
                if (line.at(0) == '.' && !(line.find_first_of(' ') < line.length() && line.substr(line.find_first_of(' '), line.length()).at(line.substr(line.find_first_of(' '), line.length()).find_first_not_of(' ')) != ';'))
                {
                    i--;
                }
            }
        }
        std::cout << "Labels done\n\n";

        i = 0;
        while (std::getline(code2, line))
        {
            if (line.length() < 2)
                continue;
            line = line.substr(line.find_first_not_of(' '), line.length());
            if (line.at(0) != ';')
            {
                parse_defines(line, i++);
            }
        }
        std::cout << "Defines done\n\n";

        i = 0;
        while (std::getline(code, line))
        {
            if (line.length() < 2 || line.at(line.find_first_not_of(' ')) == '#')
                continue;
            line = line.substr(line.find_first_not_of(' '), line.length());
            if ((line.at(0) != '.' || (line.find_first_of(' ') < line.length() && line.substr(line.find_first_of(' '), line.length()).at(line.substr(line.find_first_of(' '), line.length()).find_first_not_of(' ')) != ';')) && line.at(0) != ';')
            {
                std::cout << i << ":\n";
                parse(line, i++);
            }
        }
        std::cout << "Done!";
        std::vector<int> out;
        for (int i = 0; i < outR1.size(); i++)
        {
            std::bitset<16> bits(outR1[i]);
            out.push_back(bits.to_ulong());
        }
        for (int i = 0; i < 256-outR1.size(); i++) {
            out.push_back(0);
        }
        data["SubChips"][23]["InternalData"] = out;

        out.clear();
        for (int i = 0; i < outR2.size(); i++)
        {
            std::bitset<16> bits(outR2[i]);
            out.push_back(bits.to_ulong());
        }
        for (int i = 0; i < 256-outR2.size(); i++) {
            out.push_back(0);
        }
        data["SubChips"][10]["InternalData"] = out;

        out.clear();
        for (int i = 0; i < outR3.size(); i++)
        {
            std::bitset<16> bits(outR3[i]);
            out.push_back(bits.to_ulong());
        }
        for (int i = 0; i < 256-outR3.size(); i++) {
            out.push_back(0);
        }
        data["SubChips"][9]["InternalData"] = out;

        std::ofstream out_file("C:\\Users\\ASUS\\AppData\\LocalLow\\SebastianLague\\Digital-Logic-Sim\\Projects\\MUCH_BETTER_CPU\\Chips\\CPU.json");
        out_file << data.dump(4);
        out_file.close();
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << "\nError on line " << i;
    }

    return 0;
}