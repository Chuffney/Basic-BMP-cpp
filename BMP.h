#ifndef BMP_BMP_H
#define BMP_BMP_H

#include <string>
#include <vector>

class BMP
{
    bool initialised;
    int width;
    int height;
    int bitDepth;
    unsigned char **R;
    unsigned char **G;
    unsigned char **B;
    unsigned char **A;

    const std::string fourBytes = "00000000";

    void init(const std::string& fileName);

    static int makeBigEndian(std::vector<unsigned char>);
    static int makeBigEndian(std::vector<unsigned char> num, int start, int end);
    static std::string makeLittleEndian(int num, int bytes);
    static std::string makeLittleEndian(int num);

    static std::string extend(std::string& hexValue, int bytes);
    static std::string encode(const std::string& hexValue);
    static std::string write(int num);
    static std::string write(int num, int bytes);
    static int* digestColourTable(const std::vector<unsigned char>& colourTable);
public:
    BMP();
    explicit BMP(std::string fileName);
    ~BMP();

    enum RGBA {
        Red,
        Green,
        Blue,
        Alpha
    };

    void open(std::string);
    void close();
    void exportBMP(std::string fileName);

    void setSize(int x, int y);
    void setBitDepth(int value);
    void setColour(int x, int y, int value);
    void setColour(int x, int y, RGBA colour, int value);

    int getWidth() const;
    int getHeight() const;
    int getBitDepth() const;
    int getColour(int x, int y, RGBA colour) const;
};

#endif //BMP_BMP_H