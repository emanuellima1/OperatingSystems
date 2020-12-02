#include "../include/filesystem.hpp"
#include <iostream>
#include <fstream>
#include <ctime>


Filesystem::Filesystem(std::string filename) {
    fs = new std::fstream(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!fs->is_open()) {
        fs->open(filename, std::ios::out | std::ios::binary);
        if (!fs->is_open()) {
            std::cerr << "Não consegui abrir o arquivo " << filename << std::endl;
            exit(EXIT_FAILURE);
        }

        bitmap[0] = 0x00; // First 8 pages are used by bitmap and fat
        fs->put((char) 0x00);
        for (uint i = 1; i < MAX_PAGES/8; i++) {
            fs->put((char) 0xff);
            bitmap[i] = 0xff;
        }

        for (uint i = 0; i < MAX_PAGES; i++) {
            fs->put((char)-1);
            fat[i] = -1;
        }
        fs->close();
        fs->open(filename, std::ios::in | std::ios::out | std::ios::binary);
        mkdir("/");
    }

    else {
        for (uint i = 0; i < MAX_PAGES/8; i++)
            bitmap[i] = (uint) fs->get();
        for (uint i = 0; i < MAX_PAGES; i++)
            fat[i] = (uint) fs->get();
    }
}

Filesystem::~Filesystem() {
    delete fs;
}

void Filesystem::df() {}

bool Filesystem::bitmap_get(int i) {
    return bitmap[i/8] & (1 << (i % 8));
}

bool Filesystem::bitmap_get(int i, int k) {
    return bitmap[i] & (1 << k);
}

bool Filesystem::bitmap_set(int i, bool val) {
    if (i >= MAX_PAGES)
        return false;
    std::cout << bitmap[i] << "\n";
    if (val)
        bitmap[i/8] |= 1 << (i % 8);
    else
        bitmap[i/8] &= ~(1 << (i % 8));

    fs->seekp(i/8);
    fs->put(bitmap[i/8]);
    return true;
}

bool Filesystem::bitmap_set(int i, int k, bool val) {
    if (i*8 + k >= MAX_PAGES)
        return false;
    if (val)
        bitmap[i] = bitmap[i] | 1 << k;
    else
        bitmap[i] = bitmap[i] & ~(1 << k);
    fs->seekp(i);
    fs->put(bitmap[i]);
    return true;
}

int Filesystem::fat_set(int i, int val) {
    if (i >= MAX_PAGES)
        return -1;
    fat[i] = val;
    fs->seekp(MAX_PAGES + i);
    fs->put(val);
    return val;
}

int Filesystem::fat_get(int i) {
    if (i >= MAX_PAGES)
        return -1;
    return fat[i];
}

int Filesystem::mkdir(std::string name) {
    int p = free_page();
    uint now = (uint) time(nullptr);

    if (p == -1)
        return -1;
    fs->seekp(p*PAGE_SIZE);
    (*fs) << "d-" << name << "-" << now << "-" << now << "-" << now << "--" << std::flush;
    return p;
}

int Filesystem::free_page() {
    for (int i = 0; i < MAX_PAGES/8; i++) {
        if (bitmap[i]) {
            for (int k = 0; k < 8; k++) {
                if (bitmap_get(i, k)) {
                    bitmap_set(i, k, 0);
                    return 8*i + k;
                }
            }
        }
    }
    return -1;
}
