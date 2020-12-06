#include "../include/filesystem.hpp"
#include "../include/directory.hpp"
#include "../include/regular_file.hpp"
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

Filesystem::Filesystem(std::string filename)
{
    fs = new std::fstream(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!fs->is_open()) {
        fs->open(filename, std::ios::out | std::ios::binary);
        if (!fs->is_open()) {
            std::cerr << "Não consegui abrir o arquivo " << filename << std::endl;
            exit(EXIT_FAILURE);
        }

        // We could've used bitmap_set and fat_set here, but this way
        // we skip seekp every write
        for (uint i = 0; i < BITMAP_SIZE; i++) {
            fs->put((char) 0xff);
            bitmap[i] = (char) 0xff;
        }

        int m1 = -1;
        for (uint i = 0; i < MAX_PAGES; i++) {
            fs->write((char*) &m1, sizeof(int));
            fat[i] = -1;
        }

        for (uint i = 0; i < ROOT_PAGE; i++)
            bitmap_set(i, 0);

        fs->close();
        fs->open(filename, std::ios::in | std::ios::out | std::ios::binary);

        uint now = (uint)time(NULL);
        root = new Directory(ROOT_PAGE, "/", now, now, now);
        write_file(root);
    }

    else {
        for (uint i = 0; i < BITMAP_SIZE; i++)
            bitmap[i] = fs->get();
        for (uint i = 0; i < MAX_PAGES; i++) {
            fs->read((char *) &fat[i], sizeof(int));
        }
        root = (Directory *) read_file(ROOT_PAGE);
    }
}

Filesystem::~Filesystem() {
    delete fs;
    delete_dir(root);
}
void Filesystem::delete_dir(Directory *d) {
    for (auto& [name, t] : d->files)
        if (std::get<0>(t)) {
            if (std::get<1>(t) == 'd')
                delete_dir((Directory *) std::get<0>(t));
            else if (std::get<1>(t) == 'f')
                delete std::get<0>(t);
        }
    delete d;
}

void Filesystem::df() { }

bool Filesystem::bitmap_get(int i) {
    return bitmap[i/CHAR_BIT] & (1 << (i % CHAR_BIT));
}

bool Filesystem::bitmap_get(int i, int k)
{
    return bitmap[i] & (1 << k);
}

bool Filesystem::bitmap_set(int i, bool val)
{
    if (i >= MAX_PAGES)
        return false;
    if (val)
        bitmap[i/CHAR_BIT] |= 1 << (i % CHAR_BIT);
    else
        bitmap[i/CHAR_BIT] &= ~(1 << (i % CHAR_BIT));

    fs->seekp(i/CHAR_BIT);
    fs->put(bitmap[i/CHAR_BIT]);
    return true;
}

bool Filesystem::bitmap_set(int i, int k, bool val) {
    if (i*CHAR_BIT + k >= MAX_PAGES)
        return false;
    if (val)
        bitmap[i] = bitmap[i] | 1 << k;
    else
        bitmap[i] = bitmap[i] & ~(1 << k);
    fs->seekp(i);
    fs->put(bitmap[i]);
    return true;
}

int Filesystem::fat_set(int i, int val)
{
    if (i >= MAX_PAGES)
        return -1;
    fat[i] = val;
    fs->seekp(BITMAP_SIZE + (i * sizeof(int)));
    fs->write((char*) &val, sizeof(int));
    return val;
}

int Filesystem::fat_get(int i)
{
    if (i >= MAX_PAGES)
        return -1;
    return fat[i];
}

int Filesystem::free_page() {
    for (int i = 0; i < BITMAP_SIZE; i++) {
        if (bitmap[i]) {
            for (int k = 0; k < CHAR_BIT; k++) {
                if (bitmap_get(i, k)) {
                    bitmap_set(i, k, 0);
                    return CHAR_BIT*i + k;
                }
            }
        }
    }
    return -1;
}

File* Filesystem::get_file(std::string path)
{
    if (path == "/")
        return (File*)(root);
    while (path.back() == '/')
        path.pop_back();
    std::stringstream ss(path);
    std::string name, error_dir;
    Directory* d;
    File* f = root;



    ss.get(); // Discard first '/'
    while (std::getline(ss, name, '/')) {

        d = dynamic_cast<Directory*>(f);
        if (!d || d->files.find(name) == d->files.end())
            return nullptr;
        f = std::get<0>(d->files.at(name));
        if (!f)
            f = read_file(std::get<2>(d->files.at(name)));
    }
    return f;
}

std::string Filesystem::filename(std::string path)
{
    while (path.back() == '/')
        path.pop_back();
    std::stringstream ss(path);
    std::string name;
    while (std::getline(ss, name, '/'))
        ;

    return name;
}

std::string Filesystem::dirname(std::string name)
{
    size_t k, last = 0;
    std::string f;
    while ((k = name.find('/', last)) != std::string::npos && k != name.length() - 1)
        last = k + 1;

    if (last > 1)
        last--;
    f = name.substr(0, last);
    return f;
}

void Filesystem::mkdir(std::string path) {
    Directory *d;
    int p = free_page();
    if (p == -1)
        std::cerr << "Não há espaço disponível no sistema de arquivos. Cancelando..." << std::endl;

    Directory *parent = dynamic_cast<Directory*>(get_file(dirname(path)));
    std::string name = filename(path);
    uint now = (uint)time(NULL);

    d = new Directory(p, name, now, now, now);
    parent->files[name] = { d, 'd', p };
    write_file(d);
    write_file(parent);
}

void Filesystem::ls(std::string path)
{
    File *base = get_file(path), *f;

    std::cout << "f/d\tTamanho\tMod.\tNome" << std::endl;
    if (base && base->type == 'd') {
        for (auto& [name, t] : ((Directory*) base)->files) {
            f = std::get<0>(t);
            if (!f)
                f = std::get<0>(t) = read_file(std::get<2>(t));

            if (f->type == 'd')
                std::cout << "(d)\t" << dir_size(std::get<2>(t)) << "\t";
            else if (f->type == 'f')
                std::cout << "(f)\t" << ((RegularFile *) f)->size() << "\t";

            std::cout << f->modification_time  << "\t";
            std::cout << f->name << std::endl;
        }
    }
    else
        std::cout << base->name << std::endl;
}

RegularFile *Filesystem::touch(std::string path)
{
    RegularFile* f;
    uint now = (uint)time(NULL);

    if ((f = (RegularFile*)get_file(path))) {
        f->access_time = now;
        return f;
    }

    int p = free_page();

    if (p == -1)
        std::cerr << "Não há espaço disponível no sistema de arquivos. Cancelando..." << std::endl;

    Directory* parent = dynamic_cast<Directory*>(get_file(dirname(path)));
    std::string name = filename(path);

    f = new RegularFile(p, name, now, now, now);
    parent->files[name] = { f, 'f', p };
    write_file(f);
    write_file(parent);
    return f;
}

void Filesystem::rm(std::string path) {
    File* f;
    Directory* parent;
    if ((f = get_file(path))) {
        parent = (Directory*) get_file(dirname(path));
        parent->files.erase(f->name);
        write_file((File*) parent);

        rm((RegularFile *) f);
    }
}

void Filesystem::rm(RegularFile *f) {
    int block = f->page, next_block;
    while (block != -1) {
        next_block = fat[block];
        bitmap_set(block, 1);
        fat_set(block, -1);
        block = next_block;
    }

    delete f;
}

void Filesystem::cat(std::string path) {
    RegularFile *f;
    if ((f = dynamic_cast<RegularFile*>(get_file(path))))
        std::cout << f->content;
}

void Filesystem::cp(std::string origin, std::string path) {
    std::ifstream file(origin);
    std::stringstream ss;
    ss << file.rdbuf();

    RegularFile *f = touch(path);
    f->content = ss.str();
    write_file((File*) f);
}


void Filesystem::rmdir(std::string path) {
    Directory *d = (Directory *) get_file(path);
    if (!d || d->type != 'd')
        return;
    rmdir(d);

    Directory *parent = (Directory*) get_file(dirname(path));
    parent->files.erase(filename(path));
    write_file((File*) parent);
}

void Filesystem::rmdir(Directory * d, int n) {
    for (auto& [name, t] : d->files) {
        for (int i = 0; i < n; i++)
            std::cout << "  ";
        std::cout << "Apagando o ";
        if (std::get<1>(t) == 'd')
            std::cout << "diretório ";
        else if (std::get<1>(t) == 'f')
            std::cout << "arquivo ";
        std::cout << name << std::endl;

        if (!std::get<0>(t))
            std::get<0>(t) = read_file(std::get<2>(t));
        if (std::get<1>(t) == 'd')
            rmdir((Directory *) std::get<0>(t), n + 1);
        else if (std::get<1>(t) == 'f')
            rm((RegularFile *) std::get<0>(t));
    }

    int block = d->page, next_block;
    while (block != -1) {
        next_block = fat[block];
        bitmap_set(block, 1);
        fat_set(block, -1);
        block = next_block;
    }
    delete d;
}

void Filesystem::find(std::string path, std::string search) {
    Directory *d = (Directory*) get_file(path);
    if (!d || d->type != 'd')
        return;
    find(d, path.erase(0, 1) , search);
}

void Filesystem::find(Directory *d, std::string path, std::string search) {
    if (d->files.count(search))
        std::cout << path << "/" << search << std::endl;
    
    for (auto& [name, t] : d->files) {
        if (std::get<1>(t) == 'd') {
            if (!std::get<0>(t))
                std::get<0>(t) = (Directory *) read_file(std::get<2>(t));
            find ((Directory *) std::get<0>(t), path + '/' + name, search);
        }
    }
}

void Filesystem::write_file(File *f) {
    /* data format:
     * Separate the following fields with "-", except for the first two
     * Include one "-" at the end of all content
     *
     * * next block
     * * type (d or f)
     * * size (if type is f)
     * * name
     * * ct, mt, at
     * * content
     *
     * If type is d, the content has the following format:
     * Separate each file with "|" (including one at the end, before the
     * last "-"). Each file is composed of:
     *
     * * type
     * * block
     * * name
     *
     * without any separators (since type and block have fixed size)
     * */
    uint size;
    char type = f->type;
    std::stringstream temp;

    temp.put(type);
    temp.put('-');
    if (type == 'f') {
        size = ((RegularFile*) f)->size();
        temp.write((char*) &size, sizeof(uint));
        temp.put('-');
    }
    temp.write(f->name.c_str(), f->name.length());
    temp.put('-');
    temp.write((char*) &f->creation_time, sizeof(uint));
    temp.put('-');
    temp.write((char*) &f->modification_time, sizeof(uint));
    temp.put('-');
    temp.write((char*) &f->access_time, sizeof(uint));
    temp.put('-');
    if (type == 'd') {
        Directory* d = (Directory*)f;
        if (!d->files.empty()) {
            for (auto& [name, t] : d->files) {
                // t is a tuple<File* f,char type, int block>
                if (!std::get<0>(t))
                    std::get<0>(t) = read_file(std::get<2>(t));

                temp.put(std::get<1>(t));
                temp.write((char*) &(std::get<2>(t)), sizeof(int));
                temp.write(std::get<0>(t)->name.c_str(),
                          std::get<0>(t)->name.length());
                temp.put('|');
            }
        }
    }
    else if (type == 'f') {
        RegularFile *rf = (RegularFile*) f;
        temp.write(rf->content.c_str(), rf->content.length());
    }
    temp.put('-');

    int block, next_block = f->page;
    const uint page_space = PAGE_SIZE - sizeof(int);
    char buff[page_space];

    while (!temp.eof()) {
        if (next_block == -1) {
            next_block = free_page();
            if (next_block == -1) {
                //TODO: fazer o arquivo não existir
                std::cerr << "Sistema de arquivos cheio. Cancelando..." 
                          << std::endl;
                return;
            }
            fat_set(block, next_block);
            fat_set(next_block, -1);
        }
        block = next_block;

        fs->seekp(block * PAGE_SIZE);
        fs->write((char*) &fat[f->page], sizeof(int));

        temp.read(buff, page_space);
        fs->write(buff, page_space);

        bitmap_set(block, 0);
        next_block = fat[block];
    }

}

File* Filesystem::read_file(int page)
{
    if (page == -1)
        return nullptr;

    int block = page;
    std::stringstream temp;
    const uint page_space = PAGE_SIZE - sizeof(int);
    char buff[page_space];

    while (block != -1) {
        fs->seekg(block * PAGE_SIZE + sizeof(int));
        fs->read(buff, page_space);
        temp.write(buff, page_space);
        block = fat[block];
    }

    char c, type;
    uint size;
    std::string name;
    uint ct, mt, at;

    if ((type = temp.get()) == 'd') {
        temp.get();
        std::getline(temp, name, '-');
        temp.read((char*) &ct, sizeof(uint));
        temp.get();
        temp.read((char*) &mt, sizeof(uint));
        temp.get();
        temp.read((char*) &at, sizeof(uint));
        temp.get();
        Directory *d = new Directory(page, name, ct, mt, at);

        name.clear();
        if ((type = temp.get()) != '-') { // check if there is any file in dir
            temp.read((char*) &block, sizeof(int));
            while((c = temp.get()) != '-') {
                if (c != '|')
                    name += c;
                else {
                    d->files[name] = { nullptr, type, block };
                    name.clear();
                    if ((type = temp.get()) != '-')
                        temp.read((char*) &block, sizeof(int));
                    else
                        temp.unget();
                }
            }
        }
        return (File*)d;
    }

    else if (type == 'f') {
        temp.get();
        temp.read((char*) &size, sizeof(uint));
        temp.get();
        std::getline(temp, name, '-');
        temp.read((char*) &ct, sizeof(uint));
        temp.get();
        temp.read((char*) &mt, sizeof(uint));
        temp.get();
        temp.read((char*) &at, sizeof(uint));
        temp.get();
        RegularFile *f = new RegularFile(page, name, ct, mt, at);
        std::getline(temp, f->content, '-');
        return (File*)f;
    }

    return nullptr;
}

uint Filesystem::dir_size(int page) {
    int block = fat[page];
    uint count = 1;
    while (block != -1) {
        count++;
        block = fat[block];
    }
    return count * PAGE_SIZE;
}
