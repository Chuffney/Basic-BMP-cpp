#include "BMP.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

BMP::BMP()
{
    R = nullptr;
    G = nullptr;
    B = nullptr;
    A = nullptr;
    width = 0;
    height = 0;
    initialised = false;
    bitDepth = 24;
}

BMP::BMP(std::string fileName) : BMP()
{
    open(std::move(fileName));
}

BMP::~BMP()
{
    close();
}

void BMP::open(std::string fileName)
{
    if (!initialised)
    {
        if (!(fileName.substr(fileName.length() - 4) == ".bmp" || fileName.substr(fileName.length() - 4) == ".BMP"))
        {
            fileName += ".bmp";
        }
        init(fileName);
        initialised = true;
    }
}

void BMP::close()
{
    initialised = false;
    bitDepth = 24;
    for (int i = 0; i < width; i++)
    {
        delete[] R[i];
        delete[] G[i];
        delete[] B[i];
        delete[] A[i];
    }
    delete[] R;
    delete[] G;
    delete[] B;
    delete[] A;
    R = nullptr;
    G = nullptr;
    B = nullptr;
    A = nullptr;
    width = 0;
    height = 0;
}

void BMP::exportBMP(std::string fileName)
{
    if (!initialised)
    {
        std::cout << "Cannot export - image not initialised\n";
        return;
    }
    int fileSize;
    int rawPixelSize = (width * height * bitDepth / 8);
    int padding = (4 - ((width * (bitDepth / 8)) % 4)) % 4;
    if (!(fileName.substr(fileName.length() - 4) == ".bmp" || fileName.substr(fileName.length() - 4) == ".BMP"))
    {
        fileName += ".bmp";
    }
    switch (bitDepth)
    {
        case 24:
            fileSize = 54 + rawPixelSize + height * padding;
            break;
        case 32:
            fileSize = 74 + rawPixelSize;
            break;
        default:
            close();
            return;
    }
    std::ofstream out(fileName);
    out << "BM" << write(fileSize) << encode(fourBytes);
    switch (bitDepth)
    {
        case 24:
            out << write(54) << write(40) << write(width) << write(height) << write(1, 2) << write(bitDepth, 2);
            for (int i = 0; i < 6; i++)
            {
                out << encode(fourBytes);
            }
            break;
        case 32:
            out << write(138) << write(124) << write(width) << write(height) << write(1, 2) << write(bitDepth, 2) << write(3)
                << write(rawPixelSize) << encode("130B0000") << encode("130B0000") << encode(fourBytes) << encode(fourBytes)
                << encode("000000FF") << encode("0000FF00") << encode("00FF0000") << encode("FF000000") << "BGRs";
            for (int i = 0; i < 16; i++)
            {
                out << encode(fourBytes);
            }
            break;
    }
    for (int y = height - 1; y >= 0; y--)
    {
        for (int x = 0; x < width; x++)
        {
            if (bitDepth == 32)
            {
                out << write(A[x][y], 1);
            }
            out << write(B[x][y], 1) << write(G[x][y], 1) << write(R[x][y], 1);
        }
        for (int i = 0; i < padding; i++)
        {
            out << encode("00");
        }
    }
    out.close();
}

void BMP::init(const std::string &fileName)
{
    std::ifstream ifs;
    ifs.open(fileName);
    std::vector<unsigned char> header(0x46, 0);
    char temp;
    for (int i = 0; i < 0x1E; i++)
    {
        ifs.get(temp);
        header[i] = (unsigned char) temp;
    }
    int *colourTable;
    int offset = makeBigEndian(header, 0xA, 0xE);
    width = makeBigEndian(header, 0x12, 0x16);
    height = makeBigEndian(header, 0x16, 0x1A);
    bitDepth = makeBigEndian(header, 0x1C, 0x1E);
    R = new unsigned char *[width];
    G = new unsigned char *[width];
    B = new unsigned char *[width];
    for (int i = 0; i < width; i++)
    {
        R[i] = new unsigned char[height];
        G[i] = new unsigned char[height];
        B[i] = new unsigned char[height];
    }
    switch (bitDepth)
    {
        case 32:
            A = new unsigned char *[width];
            for (int i = 0; i < width; i++)
            {
                A[i] = new unsigned char[height];
            }
            for (int i = 0x1E; i < 0x46; i++)
            {
                ifs.get(temp);
                header[i] = (unsigned char) temp;
            }
            colourTable = digestColourTable(std::vector<unsigned char>(&header[0x36], &header[0x46]));
            for (int i = 0x46; i < offset; i++)
            {
                ifs.get(temp);
            }
            for (int y = height - 1; y >= 0; y--)
            {
                for (int x = 0; x < width; x++)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        ifs.get(temp);
                        if (colourTable[RGBA::Red] == i)
                        {
                            R[x][y] = (unsigned char) temp;
                        } else if (colourTable[RGBA::Green] == i)
                        {
                            G[x][y] = (unsigned char) temp;
                        } else if (colourTable[RGBA::Blue] == i)
                        {
                            B[x][y] = (unsigned char) temp;
                        } else if (colourTable[RGBA::Alpha] == i)
                        {
                            A[x][y] = (unsigned char) temp;
                        }
                    }
                }
            }
            break;
        case 24:
            for (int i = 0x1E; i < offset; i++)
            {
                ifs.get(temp);
            }
            int padding = (4 - ((width * 3) % 4)) % 4;
            for (int y = height - 1; y >= 0; y--)
            {
                for (int x = 0; x < width; x++)
                {
                    ifs.get(temp);
                    B[x][y] = (unsigned char) temp;
                    ifs.get(temp);
                    G[x][y] = (unsigned char) temp;
                    ifs.get(temp);
                    R[x][y] = (unsigned char) temp;
                }
                for (int i = 0; i < padding; i++)
                {
                    ifs.get(temp);
                }
            }
            break;
    }
    ifs.close();
}

void BMP::setSize(int x, int y)
{
    if (x <= 0 || y <= 0)
    {
        throw std::invalid_argument("Invalid image size (" + std::to_string(x) + ", " + std::to_string(y) + ")");
    }
    width = x;
    height = y;
    R = new unsigned char *[width];
    G = new unsigned char *[width];
    B = new unsigned char *[width];
    for (int i = 0; i < x; i++)
    {
        R[i] = new unsigned char[height];
        G[i] = new unsigned char[height];
        B[i] = new unsigned char[height];
        for (int j = 0; j < y; j++)
        {
            R[i][j] = 0;
            G[i][j] = 0;
            B[i][j] = 0;
        }
    }
    if (bitDepth == 32)
    {
        A = new unsigned char *[width];
        for (int i = 0; i < x; i++)
        {
            A[i] = new unsigned char[height];
            std::fill(&A[i][0], &A[i][height], 255);
        }
    }
    initialised = true;
}

void BMP::setBitDepth(int value)
{
    switch (value)
    {
        case 32:
            if (A == nullptr && width > 0 && height > 0)
            {
                A = new unsigned char *[width];
                for (int i = 0; i < width; i++)
                {
                    A[i] = new unsigned char[height];
                    std::fill(&A[i][0], &A[i][height], 255);
                }
            }
        case 24:
            bitDepth = value;
            break;
        default:
            throw std::invalid_argument("Invalid bit depth");
    }
}

void BMP::setColour(int x, int y, int value)
{
    setColour(x, y, RGBA::Red, value);
    G[x][y] = value;
    B[x][y] = value;
}

void BMP::setColour(int x, int y, RGBA colour, int value)
{
    if (x < 0 || y < 0 || x >= width || y >= height)
    {
        throw std::invalid_argument(
                "Specified coordinates are outside image boundaries [" + std::to_string(x) + ", " + std::to_string(y) + ']');
    } else if (value < 0 || value > 255)
    {
        throw std::invalid_argument("Colour value must be between 0 and 255");
    }
    switch (colour)
    {
        case Red:
            R[x][y] = value;
            break;
        case Green:
            G[x][y] = value;
            break;
        case Blue:
            B[x][y] = value;
            break;
        case Alpha:
            if (bitDepth == 32)
            {
                A[x][y] = value;
            } else
            {
                std::cout << "24 bpp bitmaps do not support alpha channel, to achieve transparency use 32 bpp\n";
            }
            break;
    }
}

int BMP::getWidth() const
{
    return width;
}

int BMP::getHeight() const
{
    return height;
}

int BMP::getBitDepth() const
{
    return bitDepth;
}

int BMP::getColour(int x, int y, RGBA colour) const
{
    if (x < 0 || y < 0 || x >= width || y >= height)
    {
        throw std::invalid_argument(
                "Specified coordinates are outside image boundaries [" + std::to_string(x) + ", " + std::to_string(y) + ']');
    }
    switch (colour)
    {
        case Red:
            return (int) R[x][y];
        case Green:
            return (int) G[x][y];
        case Blue:
            return (int) B[x][y];
        case Alpha:
            if (bitDepth == 32)
            {
                return A[x][y];
            } else
            {
                return 255;
            }
        default:
            return 0;
    }
}

int BMP::makeBigEndian(std::vector<unsigned char> num)
{
    int length = (int) num.size();
    int *reversed = new int[length];
    for (int i = 0; i < length; i++)
    {
        reversed[i] = num[length - i - 1];
    }
    int sum = 0;
    int mul = 1;
    for (int i = length - 1; i >= 0; i--)
    {
        sum += reversed[i] * mul;
        mul *= 256;
    }
    delete[] reversed;
    return sum;
}

int BMP::makeBigEndian(std::vector<unsigned char> num, int start, int end)
{
    std::vector<unsigned char> sub(&num[start], &num[end]);
    return makeBigEndian(sub);
}

std::string BMP::makeLittleEndian(int num, int bytes)
{
    std::string result;
    if (bytes > 4)
    {
        throw std::invalid_argument("This method does not support long int data type (max bytes = 4)");
    } else if (bytes < 0)
    {
        throw std::invalid_argument("Number of assigned bytes cannot be negative");
    }
    num = (num >> 24) | ((num << 8) & 0x00FF0000) | ((num >> 8) & 0x0000FF00) | (num << 24);
    std::stringstream stream;
    stream << std::hex << num;
    result = stream.str();
    while (result.length() < 8)
    {
        result.insert(0, "0");
    }
    return result.substr(0, bytes * 2);
}

std::string BMP::makeLittleEndian(int num)
{
    return makeLittleEndian(num, 4);
}

std::string BMP::extend(std::string &hexValue, int bytes)
{
    while (hexValue.length() < bytes * 2)
    {
        hexValue.append("0");
    }
    return hexValue;
}

std::string BMP::encode(const std::string &hexValue)
{
    std::stringstream stream;
    std::string str;
    int temp;
    for (int i = 0; i < hexValue.length(); i += 2)
    {
        stream << std::hex << hexValue.substr(i, 2);
        stream >> temp;
        stream.clear();
        str += (char) temp;
    }
    return str;
}

std::string BMP::write(int num, int bytes)
{
    return encode(makeLittleEndian(num, bytes));
}

std::string BMP::write(int num)
{
    return write(num, 4);
}

int *BMP::digestColourTable(const std::vector<unsigned char> &colourTable)
{
    static int result[4];
    for (int i = 0; i < 4; i++)
    {
        unsigned int current = makeBigEndian(colourTable, i * 4, (i * 4) + 4);
        switch (current)
        {
            case 0xFF:
                result[BMP::RGBA::Red] = i;
                break;
            case 0xFF00:
                result[BMP::RGBA::Green] = i;
                break;
            case 0xFF0000:
                result[BMP::RGBA::Blue] = i;
                break;
            case 0xFF000000:
                result[BMP::RGBA::Alpha] = i;
                break;
            default:
                throw std::logic_error("");
        }
    }
    return result;
}