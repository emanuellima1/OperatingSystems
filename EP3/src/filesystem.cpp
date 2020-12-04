#include "../include/filesystem.hpp"
#include "../include/directory.hpp"
#include "../include/regular_file.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>


Filesystem::Filesystem(std::string filename) {
    fs = new std::fstream(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!fs->is_open()) {
        fs->open(filename, std::ios::out | std::ios::binary);
        if (!fs->is_open()) {
            std::cerr << "Não consegui abrir o arquivo " << filename << std::endl;
            exit(EXIT_FAILURE);
        }
        // First 8 pages are used by bitmap and fat
        // this should be in function of ROOT_PAGE
        bitmap[0] = 0x00;
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

        uint now = (uint) time(NULL);
        root = new Directory(ROOT_PAGE, "/", -1, now, now, now, nullptr);
        write_dir(root);
    }

    else {
        for (uint i = 0; i < MAX_PAGES/8; i++)
            bitmap[i] = (uint) fs->get();
        for (uint i = 0; i < MAX_PAGES; i++)
            fat[i] = (uint) fs->get();
        root = (Directory *) read_file(ROOT_PAGE, nullptr);
    }
}

Filesystem::~Filesystem() {
    // TODO: recursively delete files
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

File* Filesystem::get_file(std::string path) {
    if (path == "/")
        return dynamic_cast<File*>(root);
    while (path.back() == '/')
        path.pop_back();
    std::stringstream ss(path);
    std::string name, error_dir;
    Directory *d;
    File *f = root;

    ss.get(); // Discard first '/'
    while(std::getline(ss, name, '/')) {
        d = dynamic_cast<Directory*>(f);
        if (!d || d->files.find(name) == d->files.end()) {
            std::cerr << "Não consigo achar o arquivo " << path << std::endl;
            return nullptr;
        }
        f = std::get<0>(d->files.at(name));
    }
    return f;
}

std::string Filesystem::filename(std::string path) {
    while (path.back() == '/')
        path.pop_back();
    std::stringstream ss(path);
    std::string name;
    while(std::getline(ss, name, '/'));

    return name;
}

std::string Filesystem::dirname(std::string name) {
    size_t k, last = 0;
    std::string f;
    while((k = name.find('/', last)) != std::string::npos &&
            k != name.length() - 1)
        last = k + 1;

    if (last > 1)
        last--;
    f = name.substr(0, last);
    return f;
}

void Filesystem::mkdir(std::string path) {
    //TODO: Se o tamanho do parente ultrapassar 4K, usar um novo bloco
    //TODO: acertar o fat e o bitmap depois de criar
    Directory *d;
    int p = free_page();
    if (p == -1)
        std::cerr << "Não há espaço disponível no sistema de arquivos. Cancelando..."<< std::endl;

    Directory *parent = dynamic_cast<Directory*>(get_file(dirname(path)));
    std::string name = filename(path);
    uint now = (uint) time(NULL);

    d = new Directory(p, name, -1, now, now, now, parent);
    parent->files[name] = {d, 'd', p};
    write_dir(d);
    write_dir(parent);
}

void Filesystem::ls(std::string path) {
    File *f = get_file(path);
    Directory *d;
    
    if ((d = dynamic_cast<Directory*>(f)))
        d->ls();
    else
        std::cout << f->name << std::endl;
}

void Filesystem::write_dir(Directory *d) {
    /* directory metadata format: 
     *
     * Or, with regex:
     * */

    fs->seekp(d->page * PAGE_SIZE);
    fs->put((char) d->next_block);
    fs->write("-d-", 3);
    fs->write(d->name.c_str(), d->name.length());
    fs->put('-');
    fs->write((char*) &d->creation_time, sizeof(uint));
    fs->put('-');
    fs->write((char*) &d->modification_time, sizeof(uint));
    fs->put('-');
    fs->write((char*) &d->access_time, sizeof(uint));
    fs->put('-');
    if (!d->files.empty()) {
        for (auto& [name, t] : d->files) {
            fs->put(std::get<1>(t));
            fs->write((char*) &(std::get<2>(t)), sizeof(int));
            fs->write(std::get<0>(t)->name.c_str(), std::get<0>(t)->name.length());
            fs->put('|');
        }
    }
    fs->put('-');
}

File* Filesystem::read_file(int page, Directory *parent) {
    char c, type;
    int next_block, block;
    uint size;
    std::string name;
    uint ct, mt, at;

    fs->seekg(page*PAGE_SIZE);
    next_block = (int) fs->get();
    fs->get();
    if (fs->get() == 'd') {
        fs->get();
        std::getline(*fs, name, '-');
        fs->read((char*) &ct, sizeof(uint));
        fs->get();
        fs->read((char*) &mt, sizeof(uint));
        fs->get();
        fs->read((char*) &at, sizeof(uint));
        fs->get();
        Directory *d = new Directory(page, name, next_block, ct, mt, at, parent);

        name.clear();
        type = fs->get();
        fs->read((char*) &block, sizeof(int));
        while((c = fs->get()) != '-') {
            if (c != '|')
                name += c;
            else {
                d->files[name] = {nullptr, type, block};
                name.clear();
                type = fs->get();
                fs->read((char*) &block, sizeof(int));
            }
        }
        return (File *) d;
    }

    else if (fs->get() == 'f') {
        fs->get();
        std::getline(*fs, name, '-');
        fs->read((char*) &size, sizeof(uint));
        fs->get();
        fs->read((char*) &ct, sizeof(uint));
        fs->get();
        fs->read((char*) &mt, sizeof(uint));
        fs->get();
        fs->read((char*) &at, sizeof(uint));
        fs->get();
        RegularFile *f = new RegularFile(page, name, next_block, ct, mt, at, parent);
        fs->get();
        name.clear();
        std::getline(*fs, f->content, '-');
        return (File *) f;
    }
    return nullptr;
}


