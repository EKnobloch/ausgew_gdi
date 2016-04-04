#include <iostream>

#include <cstdint> //for typedef
#include <fstream> //for filestreams

typedef uint8_t  CHAR;
typedef uint16_t WORD;
typedef uint32_t DWORD;

typedef int8_t  BYTE;
typedef int16_t SHORT;
typedef int32_t LONG;

typedef LONG INT;
typedef INT BOOL;

struct BMPFileHeader{
    
    WORD bfType;
    
    DWORD bfSize;
    DWORD bfReserved;
    DWORD bfOffBits;
    
};

struct BMPInfoHeader{
    
    DWORD biSize;
    
    LONG biWidth;
    LONG biHeight;
    
    WORD biPlanes;
    WORD biBitCount;
    
    DWORD biCompression;
    DWORD biSizeImage;
    
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    
    DWORD biClrUsed;
    DWORD biClrImportant;
    
};

void SaveBMP(BYTE* pBitmapBits, LONG lWidth, LONG lHeight, WORD wBitsPerPixel, const unsigned long& padding_size, std::string filepath){
    
    unsigned long headers_size = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    unsigned long pixel_data_size = lHeight * ((lWidth*(wBitsPerPixel/8)) + padding_size);
    
    BMPInfoHeader bmpi_header = {0};
    BMPFileHeader bmpf_header = {0};
    
    std::fstream write;
    
    bmpi_header.biSize = sizeof(BMPInfoHeader);
    bmpi_header.biBitCount = wBitsPerPixel;
    bmpi_header.biClrImportant = 0;
    bmpi_header.biClrUsed = 0;
    bmpi_header.biCompression = 0; // 0 == BI_RGB
    bmpi_header.biHeight = lHeight;
    bmpi_header.biWidth = lWidth;
    bmpi_header.biPlanes = 1;
    bmpi_header.biSizeImage = pixel_data_size;
    
    bmpf_header.bfType = 0x4D42;
    bmpf_header.bfOffBits = headers_size;
    bmpf_header.bfSize = headers_size + pixel_data_size;
    
    write.open(filepath.c_str(), std::ios::out | std::ios::binary);
    
    if(write.fail()){
        
        std::cout << "[!] failed to write to file!" << std::endl;
        
        return;
        
    }
    
    write.write((char*)&bmpf_header, sizeof(bmpf_header));
    
    write.write((char*)&bmpi_header, sizeof(bmpi_header));
    
    write.write((char*)pBitmapBits, bmpi_header.biSizeImage);
    
    /*
    write.write(reinterpret_cast<char *>(&bmpf_header.bfType), sizeof(bmpf_header.bfType));
    write.write(reinterpret_cast<char *>(&bmpf_header.bfSize), sizeof(bmpf_header.bfSize));
    write.write(reinterpret_cast<char *>(&bmpf_header.bfReserved), sizeof(bmpf_header.bfReserved));
    write.write(reinterpret_cast<char *>(&bmpf_header.bfOffBits), sizeof(bmpf_header.bfOffBits));
    
    write.write(reinterpret_cast<char *>(&bmpi_header.biSize), sizeof(bmpi_header.biSize));
    write.write(reinterpret_cast<char *>(&bmpi_header.biWidth), sizeof(bmpi_header.biWidth));
    write.write(reinterpret_cast<char *>(&bmpi_header.biHeight), sizeof(bmpi_header.biHeight));
    write.write(reinterpret_cast<char *>(&bmpi_header.biPlanes), sizeof(bmpi_header.biPlanes));
    write.write(reinterpret_cast<char *>(&bmpi_header.biBitCount), sizeof(bmpi_header.biBitCount));
    write.write(reinterpret_cast<char *>(&bmpi_header.biCompression), sizeof(bmpi_header.biCompression));
    write.write(reinterpret_cast<char *>(&bmpi_header.biSizeImage), sizeof(bmpi_header.biSizeImage));
    write.write(reinterpret_cast<char *>(&bmpi_header.biXPelsPerMeter), sizeof(bmpi_header.biXPelsPerMeter));
    write.write(reinterpret_cast<char *>(&bmpi_header.biYPelsPerMeter), sizeof(bmpi_header.biYPelsPerMeter));
    write.write(reinterpret_cast<char *>(&bmpi_header.biClrUsed), sizeof(bmpi_header.biClrUsed));
    write.write(reinterpret_cast<char *>(&bmpi_header.biClrImportant), sizeof(bmpi_header.biClrImportant));
     */
     
    //write.write(reinterpret_cast<char *>(pBitmapBits), bmpi_header.biSizeImage);
    
    
    write.close();
    
}

std::unique_ptr<BYTE[]> CreateNewBuffer(unsigned long& padding, BYTE* pmatrix, const int& width, const int& height){
    
    int scanlinebytes, total_scanlinebytes;
    
    long newsize, bufpos = 0, newpos = 0;
    
    padding = (4 - ((width * 3)%4))%4;
    
    scanlinebytes = width * 3;
    total_scanlinebytes = scanlinebytes + padding;
    
    newsize = height * total_scanlinebytes;
    
    std::unique_ptr<BYTE[]> newbuf(new BYTE[newsize]);
    
    //fill new array with original buffer, pad remaining with zeros
    
    std::fill(&newbuf[0], &newbuf[newsize], 0);
    
    std::cout << "[i] new buffer of size " << newsize << " created." << std::endl;
    
    for(int y = 0; y < height; y++){
        
        for(int x = 0; x < 3 * width; x+=3){
            
            //Determine positions in original and padded buffers
            
            bufpos = y * 3 * width + (3 * width - x);
            newpos = (height - y - 1) * total_scanlinebytes + x;
            
            //swap R&B, G remains, swap B&R
            
            newbuf[newpos] = pmatrix[bufpos + 2];
            newbuf[newpos + 1] = pmatrix[bufpos + 1];
            newbuf[newpos + 2] = pmatrix[bufpos];
            
        }
        
    }
    
    return newbuf;
    
}

BYTE* LoadBMP(int* width, int* height, unsigned long* size, std::string bmpfile){
    
    //reading in bmp structs
    
    BMPFileHeader bmpf_header;
    BMPInfoHeader bmpi_header;
    
    std::ifstream read;
    read.open(bmpfile.c_str(), std::ios::in | std::ios::binary);
    
    if(read.fail()){
        
        std::cout << "[!] Could not open bmp file!" << std::endl;
        return NULL;
        
    }
    
    read.read(reinterpret_cast<char *>(&bmpf_header.bfType), sizeof(bmpf_header.bfType));
    read.read(reinterpret_cast<char *>(&bmpf_header.bfSize), sizeof(bmpf_header.bfSize));
    read.read(reinterpret_cast<char *>(&bmpf_header.bfReserved), sizeof(bmpf_header.bfReserved));
    read.read(reinterpret_cast<char *>(&bmpf_header.bfOffBits), sizeof(bmpf_header.bfOffBits));
    
    read.read(reinterpret_cast<char *>(&bmpi_header.biSize), sizeof(bmpi_header.biSize));
    read.read(reinterpret_cast<char *>(&bmpi_header.biWidth), sizeof(bmpi_header.biWidth));
    read.read(reinterpret_cast<char *>(&bmpi_header.biHeight), sizeof(bmpi_header.biHeight));
    read.read(reinterpret_cast<char *>(&bmpi_header.biPlanes), sizeof(bmpi_header.biPlanes));
    read.read(reinterpret_cast<char *>(&bmpi_header.biBitCount), sizeof(bmpi_header.biBitCount));
    read.read(reinterpret_cast<char *>(&bmpi_header.biCompression), sizeof(bmpi_header.biCompression));
    read.read(reinterpret_cast<char *>(&bmpi_header.biSizeImage), sizeof(bmpi_header.biSizeImage));
    read.read(reinterpret_cast<char *>(&bmpi_header.biXPelsPerMeter), sizeof(bmpi_header.biXPelsPerMeter));
    read.read(reinterpret_cast<char *>(&bmpi_header.biYPelsPerMeter), sizeof(bmpi_header.biYPelsPerMeter));
    read.read(reinterpret_cast<char *>(&bmpi_header.biClrUsed), sizeof(bmpi_header.biClrUsed));
    read.read(reinterpret_cast<char *>(&bmpi_header.biClrImportant), sizeof(bmpi_header.biClrImportant));
    
    if(bmpf_header.bfType != 'MB'){
        
        std::cout << "[!] not a valid bmp format!" << std::endl;
        
        read.close();
        return NULL;
        
    }
    
    //setting image measurements
    
    *width = bmpi_header.biWidth;
    *height = abs(bmpi_header.biHeight);
    
    //check if bmp is uncompressed
    
    if(bmpi_header.biCompression != 0){
        
        std::cout << "[!] bmp seems to be compressed." << std::endl;
        
        read.close();
        return NULL;
        
    }
    
    //check if bmp uses 24 bit map
    
    if(bmpi_header.biBitCount != 24){
        
        std::cout << "[!] bmp does not use 24 bit map." << std::endl;
        
        read.close();
        return NULL;
        
    }
    
    //create byte buffer for image data
    
    *size = bmpf_header.bfSize - bmpf_header.bfOffBits;
    BYTE* buffer = new BYTE[*size];
    char temp_buffer[*size];
    
    //move filepointer to start of image data
    
    read.seekg(bmpf_header.bfOffBits);
    
    //read image data
    
    read.read(temp_buffer, *size);
    
    for(int i=0; i < *size; i++){
        
        buffer[i] = temp_buffer[i];
        
    }
    
    //done
    
    read.close();
    
    return buffer;
    
}

int main(int argc, const char * argv[]) {
    
    std::string bmp_path = "/Users/maxmustermann/Downloads/shaunak.bmp";
    
    int height = 0;
    int width = 0;
    
    unsigned long size = 0, padding = 0;
    
    //std::cout << "[i] path to bmp (without spaces): ";
    //std::cin >> bmp_path;
    //std::cout << std::endl;
    
    BYTE* bin_buffer = LoadBMP(&width, &height, &size, bmp_path);
    
    for(int i = 0; i < size; i++){
        
        //std::cout << bin_buffer[i];
        
    }
    
    std::cout << std::endl;
    
    //output
    
    std::reverse(bin_buffer, bin_buffer + size);
    
    std::unique_ptr<BYTE[]> newbuf2 = CreateNewBuffer(padding, bin_buffer, width, height);
    
    std::cout << "[i] old size was " << size << "." << std::endl;
    
    SaveBMP((BYTE*) &newbuf2[0], width, height, 24, padding, "/tmp/out.bmp");
    
    return 0;
    
}
