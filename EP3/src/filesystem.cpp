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
        for (uint i = 0; i < MAX_PAGES/8; i++) {
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
        root = new Directory(ROOT_PAGE, "/", -1, now, now, now);
        bitmap_set(ROOT_PAGE, 0);
        write_file(root);
    }

    else {
        for (uint i = 0; i < MAX_PAGES/8; i++)
            bitmap[i] = fs->get();
        for (uint i = 0; i < MAX_PAGES; i++)
            fs->read((char *) &fat[i], sizeof(int));
        root = (Directory *) read_file(ROOT_PAGE);
    }
}

Filesystem::~Filesystem()
{
    // TODO: recursively delete files
    delete fs;
}

void Filesystem::df() { }

bool Filesystem::bitmap_get(int i)
{
    return bitmap[i / 8] & (1 << (i % 8));
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
        bitmap[i / 8] |= 1 << (i % 8);
    else
        bitmap[i / 8] &= ~(1 << (i % 8));

    fs->seekp(i / 8);
    fs->put(bitmap[i / 8]);
    return true;
}

bool Filesystem::bitmap_set(int i, int k, bool val)
{
    if (i * 8 + k >= MAX_PAGES)
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
    fs->seekp(MAX_PAGES + (i * sizeof(int)));
    fs->write((char*) &val, sizeof(int));
    return val;
}

int Filesystem::fat_get(int i)
{
    if (i >= MAX_PAGES)
        return -1;
    return fat[i];
}

int Filesystem::free_page()
{
    for (int i = 0; i < MAX_PAGES / 8; i++) {
        if (bitmap[i]) {
            for (int k = 0; k < 8; k++) {
                if (bitmap_get(i, k)) {
                    bitmap_set(i, k, 0);
                    return 8 * i + k;
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

    Directory* parent = dynamic_cast<Directory*>(get_file(dirname(path)));
    std::string name = filename(path);
    uint now = (uint)time(NULL);

    d = new Directory(p, name, -1, now, now, now);
    parent->files[name] = { d, 'd', p };
    write_file(d);
    write_file(parent);
    bitmap_set(p, 0);
}

void Filesystem::ls(std::string path)
{
    File* f = get_file(path);
    Directory* d;

    if (f->type == 'd')
        (d = (Directory*)f)->ls();
    else
        std::cout << f->name << std::endl;
}

void Filesystem::touch(std::string path)
{
    RegularFile* f;
    uint now = (uint)time(NULL);

    if ((f = (RegularFile*)get_file(path))) {
        f->access_time = now;
        return;
    }

    int p = free_page();

    if (p == -1)
        std::cerr << "Não há espaço disponível no sistema de arquivos. Cancelando..." << std::endl;

    Directory* parent = dynamic_cast<Directory*>(get_file(dirname(path)));
    std::string name = filename(path);

    f = new RegularFile(p, name, -1, now, now, now);
    parent->files[name] = { f, 'f', p };
    write_file(f);
    write_file(parent);
    bitmap_set(p, 0);
}

void Filesystem::rm(std::string path)
{
    File* f;
    Directory* parent;
    if ((f = get_file(path))) {
        /* if ((f = dynamic_cast<RegularFile*>(get_file(path)))) { */

        parent = (Directory*) get_file(dirname(path));
        parent->files.erase(f->name);
        bitmap_set(f->page, 1);
        delete f;
    }
    std::cout << f << "\n";
}

void Filesystem::write_file(File* f)
{
    //TODO: Se o tamanho do parente ultrapassar 4K, usar um novo bloco

=======

}


void Filesystem::write_file(File *f) {
>>>>>>> 0fd6a19 (EP3: Store FAT entries as ints)
    /* data format:
     * Separate the following fields with "-" (including one at the
     * end):
     *
     * * next block
     * * type (d or f)
     * * size (if type is f)
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

    temp.write((char*) &f->next_block, sizeof(int));
    temp.put('-');
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

    int block = f->page, next_block;
    char buff[PAGE_SIZE];
    uint remaining = temp.str().length() - (uint) temp.tellg();

    while (remaining > PAGE_SIZE) {
        next_block = free_page();
        if (next_block == -1) {
            std::cerr << "Sistema de arquivos cheio. Cancelando..." << std::endl;
            return;
        }
        fat_set(block, next_block);
        bitmap_set(next_block, 0);

        fs->seekp(block * PAGE_SIZE);
        temp.read(buff, PAGE_SIZE);
        fs->write(buff, PAGE_SIZE);

        fs->seekp(block * PAGE_SIZE);
        fs->write((char*) &next_block, sizeof(int));

        block = next_block;
        remaining -= PAGE_SIZE;
    }
    fs->seekp(block * PAGE_SIZE);
    temp.read(buff, remaining);
    fs->write(buff, remaining);
}

File* Filesystem::read_file(int page)
{
    if (page == -1)
        return nullptr;

    int block = page;
    std::stringstream temp;
    char buff[PAGE_SIZE];

    while (block != -1) {
        fs->seekg(block * PAGE_SIZE);
        fs->read(buff, PAGE_SIZE);
        temp.write(buff, PAGE_SIZE);
        block = fat[block];
    }

    char c, type;
    int next_block;
    uint size;
    std::string name;
    uint ct, mt, at;

    temp.read((char *) &next_block, sizeof(int));
    temp.get();

    if (temp.get() == 'd') {
        temp.get();
        std::getline(temp, name, '-');
        temp.read((char*) &ct, sizeof(uint));
        temp.get();
        temp.read((char*) &mt, sizeof(uint));
        temp.get();
        temp.read((char*) &at, sizeof(uint));
        temp.get();
        Directory *d = new Directory(page, name, next_block, ct, mt, at);

        name.clear();
        if ((type = temp.get()) != '-') { // check if there is any file in dir
            temp.read((char*) &block, sizeof(int));
            while((c = temp.get()) != '-') {
                if (c != '|')
                    name += c;
                else {
                    d->files[name] = { nullptr, type, block };
                    name.clear();
                    type = temp.get();
                    temp.read((char*) &block, sizeof(int));
                    if ((type = temp.get()) != '-')
                        temp.read((char*) &block, sizeof(int));
                    else
                        temp.unget();
                }
            }
        }
        return (File*)d;
    }

    else if (temp.get() == 'f') {
        temp.get();
        std::getline(*fs, name, '-');
        temp.read((char*) &size, sizeof(uint));
        temp.get();
        temp.read((char*) &ct, sizeof(uint));
        temp.get();
        temp.read((char*) &mt, sizeof(uint));
        temp.get();
        temp.read((char*) &at, sizeof(uint));
        temp.get();
        RegularFile *f = new RegularFile(page, name, next_block, ct, mt, at);
        temp.get();
        name.clear();
        std::getline(*fs, f->content, '-');
        return (File*)f;
    }
    return nullptr;
}
