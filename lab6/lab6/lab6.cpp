#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include "LogBuffer.h"

#pragma comment(lib, "winmm.lib")

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#undef min
#undef max

DWORD startTime = 0;

typedef struct
{
    uint8_t r, g, b, a;
} rgb32;


#if !defined(_WIN32) && !defined(_WIN64)
#pragma pack(2)
typedef struct
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;
#pragma pack()


#pragma pack(2)
typedef struct
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int16_t biXPelsPerMeter;
    int16_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;
#pragma pack()
#endif

#pragma pack(2)
typedef struct
{
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
} BMPINFO;
#pragma pack()


class bitmap
{
private:
    BMPINFO bmpInfo;
    uint8_t* pixels;

public:
    bitmap(const char* path);
    ~bitmap();

    void save(const char* path, uint16_t bit_count = 24);

    rgb32* getPixel(uint32_t x, uint32_t y) const;

    uint32_t getWidth() const;
    uint32_t getHeight() const;
};

bitmap::bitmap(const char* path) : bmpInfo(), pixels(nullptr)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);

    if (file)
    {
        file.read(reinterpret_cast<char*>(&bmpInfo.bfh), sizeof(bmpInfo.bfh));

        if (bmpInfo.bfh.bfType != 0x4d42)
        {
            throw std::runtime_error("Invalid format. Only bitmaps are supported.");
        }

        file.read(reinterpret_cast<char*>(&bmpInfo.bih), sizeof(bmpInfo.bih));

        if (bmpInfo.bih.biCompression != 0)
        {
            std::cerr << bmpInfo.bih.biCompression << "\n";
            throw std::runtime_error("Invalid bitmap. Only uncompressed bitmaps are supported.");
        }

        if (bmpInfo.bih.biBitCount != 24 && bmpInfo.bih.biBitCount != 32)
        {
            throw std::runtime_error("Invalid bitmap. Only 24bit and 32bit bitmaps are supported.");
        }

        file.seekg(bmpInfo.bfh.bfOffBits, std::ios::beg);

        pixels = new uint8_t[bmpInfo.bfh.bfSize - bmpInfo.bfh.bfOffBits];
        file.read(reinterpret_cast<char*>(&pixels[0]), bmpInfo.bfh.bfSize - bmpInfo.bfh.bfOffBits);

        uint8_t* temp = new uint8_t[bmpInfo.bih.biWidth * bmpInfo.bih.biHeight * sizeof(rgb32)];

        uint8_t* in = pixels;
        rgb32* out = reinterpret_cast<rgb32*>(temp);
        int padding = bmpInfo.bih.biBitCount == 24 ? ((bmpInfo.bih.biSizeImage - bmpInfo.bih.biWidth * bmpInfo.bih.biHeight * 3) / bmpInfo.bih.biHeight) : 0;

        for (int i = 0; i < bmpInfo.bih.biHeight; ++i, in += padding)
        {
            for (int j = 0; j < bmpInfo.bih.biWidth; ++j)
            {
                out->b = *(in++);
                out->g = *(in++);
                out->r = *(in++);
                out->a = bmpInfo.bih.biBitCount == 32 ? *(in++) : 0xFF;
                ++out;
            }
        }

        delete[] pixels;
        pixels = temp;
    }
}

bitmap::~bitmap()
{
    delete[] pixels;
}

void bitmap::save(const char* path, uint16_t bit_count)
{
    std::ofstream file(path, std::ios::out | std::ios::binary);

    if (file)
    {
        bmpInfo.bih.biBitCount = bit_count;
        uint32_t size = ((bmpInfo.bih.biWidth * bmpInfo.bih.biBitCount + 31) / 32) * 4 * bmpInfo.bih.biHeight;
        bmpInfo.bfh.bfSize = bmpInfo.bfh.bfOffBits + size;

        file.write(reinterpret_cast<char*>(&bmpInfo.bfh), sizeof(bmpInfo.bfh));
        file.write(reinterpret_cast<char*>(&bmpInfo.bih), sizeof(bmpInfo.bih));
        file.seekp(bmpInfo.bfh.bfOffBits, std::ios::beg);

        uint8_t* out = NULL;
        rgb32* in = reinterpret_cast<rgb32*>(pixels);
        uint8_t* temp = out = new uint8_t[bmpInfo.bih.biWidth * bmpInfo.bih.biHeight * sizeof(rgb32)];
        int padding = bmpInfo.bih.biBitCount == 24 ? ((bmpInfo.bih.biSizeImage - bmpInfo.bih.biWidth * bmpInfo.bih.biHeight * 3) / bmpInfo.bih.biHeight) : 0;

        for (int i = 0; i < bmpInfo.bih.biHeight; ++i, out += padding)
        {
            for (int j = 0; j < bmpInfo.bih.biWidth; ++j)
            {
                *(out++) = in->b;
                *(out++) = in->g;
                *(out++) = in->r;

                if (bmpInfo.bih.biBitCount == 32)
                {
                    *(out++) = in->a;
                }
                ++in;
            }
        }

        file.write(reinterpret_cast<char*>(&temp[0]), size); //bmpInfo.bfh.bfSize - bmpInfo.bfh.bfOffBits
        delete[] temp;
    }
}

rgb32* bitmap::getPixel(uint32_t x, uint32_t y) const
{
    rgb32* temp = reinterpret_cast<rgb32*>(pixels);
    return &temp[(bmpInfo.bih.biHeight - 1 - y) * bmpInfo.bih.biWidth + x];
}

uint32_t bitmap::getWidth() const
{
    return bmpInfo.bih.biWidth;
}

uint32_t bitmap::getHeight() const
{
    return bmpInfo.bih.biHeight;
}

struct Bmp {
    bitmap* bmp_first;
    bitmap* bmp_second;
    int little_weight;
    int little_height;
    int index;
    int count;
    uint32_t start_height;
    uint32_t end_height;
    uint32_t start_width;
    uint32_t end_width;
    std::ofstream* file;
    int thread_number;
    LogBuffer* logBuffer;
};

void blur(bitmap* bmp_first, bitmap* bmp_second, int radius, Bmp* bmp)
{
    float rs = ceil(radius * 2.57);
    for (int i = bmp->start_height; i < bmp->end_height; ++i)
    {
        for (int j = bmp->start_width; j < bmp->end_width; ++j)
        {
            double r = 0, g = 0, b = 0;
            double count = 0;

            for (int iy = i - rs; iy < i + rs + 1; ++iy)
            {
                for (int ix = j - rs; ix < j + rs + 1; ++ix)
                {
                    auto x = std::min(static_cast<int>(bmp->end_width) - 1, std::max(0, ix));
                    auto y = std::min(static_cast<int>(bmp->end_height) - 1, std::max(0, iy));

                    auto dsq = ((ix - j) * (ix - j)) + ((iy - i) * (iy - i));
                    auto wght = std::exp(-dsq / (2.0 * radius * radius)) / (3.14 * 2.0 * radius * radius);

                    rgb32* pixel = bmp_first->getPixel(x, y);

                    r += pixel->r * wght;
                    g += pixel->g * wght;
                    b += pixel->b * wght;
                    count += wght;
                }
            }

            rgb32* pixel = bmp_second->getPixel(j, i);

            pixel->r = std::round(r / count);
            pixel->g = std::round(g / count);
            pixel->b = std::round(b / count);

            int time = timeGetTime() - startTime;
            bmp->logBuffer->Log(time);
            *bmp->file << bmp->thread_number << " " << time << std::endl;
        }
    }
}

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    struct Bmp* bmp = (struct Bmp*)lpParam;

    for (int i = 0; i < bmp->count; i++)
    {
        bmp->start_height = bmp->little_height * i;
        bmp->end_height = bmp->little_height * (i + 1);
        bmp->start_width = bmp->index * bmp->little_weight;
        bmp->end_width = (bmp->index + 1) * bmp->little_weight;
        blur(bmp->bmp_first, bmp->bmp_second, 5, bmp);
    }

    ExitThread(0); // функция устанавливает код завершения потока в 0
}

int main(int argc, const char* argv[])
{
    startTime = timeGetTime();

    if (strcmp(argv[1], "/") == 0) {
        std::cout << "Arguments example: C:\\Users\\Anna\\Desktop\\image.bmp C:\\Users\\Anna\\Desktop\\image-save.bmp 3 1 0 0 0 \n"
            "1 argument : input file \n"
            "2 argument : output file \n"
            "3 argument : threads count \n"
            "4 argument : cores number \n"
            "Priority:\n `-1` - below_normal;\n `0` - normal;\n `1` - above_normal \n"
            "5 argument : first thread priority \n"
            "6 argument : second thread priority \n"
            "7 argument : third thread priority" << std::endl;

        exit(0);
    }

    bitmap bmp1{ argv[1] };
    bitmap bmp2{ argv[1] };

    int count = atoi(argv[3]);

    int littleWidth = bmp1.getWidth() / count;
    int littleHeight = bmp1.getHeight() / count;

    std::ofstream* files = new std::ofstream[count];
    LogBuffer logBuffer;

    Bmp* arrayBmp = new Bmp[count];
    for (int i = 0; i < count; i++) {
        Bmp bmp;
        std::string name = "output" + std::to_string(i) + ".txt";
        files[i] = std::ofstream(name);
        bmp.bmp_first = &bmp1;
        bmp.bmp_second = &bmp2;
        bmp.index = i;
        bmp.little_weight = littleWidth;
        bmp.little_height = littleHeight;
        bmp.count = count;
        bmp.file = &files[i];
        bmp.thread_number = i;
        bmp.logBuffer = &logBuffer;
        arrayBmp[i] = bmp;
    }

    int* priorities = new int[count];
    for (int i = 0; i < count; i++) {
        priorities[i] = atoi(argv[i + 5]);
    }

    HANDLE* handles = new HANDLE[count];
    for (int i = 0; i < count; i++) {
        handles[i] = CreateThread(NULL, 0, &ThreadProc, &arrayBmp[i], CREATE_SUSPENDED, NULL);
        SetThreadAffinityMask(handles[i], (1 << atoi(argv[4])) - 1);
        SetThreadPriority(handles[i], priorities[i]);
    }

    for (int i = 0; i < count; i++) {
        ResumeThread(handles[i]);
    }

    WaitForMultipleObjects(count, handles, true, INFINITE);

    bmp2.save(argv[2]);

    return 0;
}