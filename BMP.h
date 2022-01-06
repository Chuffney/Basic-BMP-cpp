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

    std::string outputString;

    void init(const std::string& fileName);

    void write(int num, int bytes = 4);

    static int makeBigEndian(const std::vector<unsigned char> &num);
    static int makeBigEndian(const std::vector<unsigned char> &num, int start, int end);
    static unsigned int swapEndianness(unsigned int num);

    void encode(unsigned int num, int bytes = 4);
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