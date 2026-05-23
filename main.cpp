#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <queue>
#include <cstdint>

struct node
{
    uint64_t f = 0;
    char data = '\0';
    node *left = nullptr;
    node *right = nullptr;
};
struct compare
{
    bool operator()(node *a, node *b)
    {
        return a->f > b->f;
    }
};

void map_path(node *head, std::map<char, std::string> &huffmap, std::ofstream &out, std::string path = "")
{
    if (!head)
        return;

    if (!head->left && !head->right)
    {
        out << 1 << head->data;
        huffmap[head->data] = path;
    }
    else
    {
        out << 0;
    }
    map_path(head->left, huffmap, out, path + "0");
    map_path(head->right, huffmap, out, path + "1");
}

uint8_t cursor = 128;
uint8_t x = 0;
void flush(std::ofstream &out)
{
    if (cursor == 128)
        return;
    out.write(
        reinterpret_cast<char *>(&x),
        sizeof(x));
    x = 0;
    cursor = 128;
}
void accumulator(bool val, std::ofstream &out)
{
    x += cursor * val;
    cursor >>= 1;
    if (cursor == 0)
    {
        flush(out);
    }
}

int compress(std::string input, std::string output)
{
    std::ifstream in(input, std::ios::binary);
    std::ofstream out(output, std::ios::binary);
    std::map<char, uint64_t> mp;

    std::priority_queue<node *, std::vector<node *>, compare> pq;

    if (!in.is_open())
    {
        return 1; // file failed to open
    }
    if (!out.is_open())
    {
        return 2;
    }

    // creating frequency table;
    char c;
    while (in.get(c))
    {
        mp[c]++;
    }

    // table to nodes;
    int count = 0;
    for (auto &i : mp)
    {
        node *t = new node();
        t->data = i.first;
        t->f = i.second;
        pq.push(t);
        count++;
    }

    // build tree;
    while (pq.size() > 1)
    {
        node *a = pq.top();
        pq.pop();
        node *b = pq.top();
        pq.pop();

        node *t = new node();
        t->f = a->f + b->f;
        t->left = a;
        t->right = b;
        pq.push(t);
    }
    if (pq.size() == 0)
    {
        pq.push(new node());
    }

    node *head = pq.top();

    std::string extension = "";
    for (size_t i = input.size(); i-- > 0;)
    {
        if (input[i] == '.')
        {
            break;
        }
        else
        {
            extension = input[i] + extension;
        }
    }
    if (input == extension)
    {
        extension = "";
    }

    uint8_t ext_size = extension.length();
    if (ext_size != extension.length())
    {
        ext_size == 0;
    }

    out.write(reinterpret_cast<char *>(&ext_size), sizeof(ext_size));

    for (int i = 0; i < ext_size; i++)
    {
        out << extension[i];
    }

    for (size_t i = 0; i < count; i++)
    {
        /* code */
    }

    uint64_t file_size = head->f;
    out.write(reinterpret_cast<char *>(&file_size), sizeof(file_size));

    // for quick encoding
    std::map<char, std::string> huffmap;

    // populates huffman also writes map to file in preorder
    map_path(head, huffmap, out);

    in.clear();
    in.seekg(0);

    while (in.get(c))
    {
        for (char i : huffmap[c])
        {
            if (i == '1')
            {
                accumulator(true, out);
            }
            else if (i == '0')
            {
                accumulator(false, out);
            }
        }
    }
    flush(out);

    return 0;
}

node *readtree(std::ifstream &in)
{
    node *t = new node();
    char c;
    if (!in.get(c))
        return nullptr;
    if (c == '0')
    {
        t->left = readtree(in);
        t->right = readtree(in);
    }
    else if (c == '1')
    {
        if (!in.get(c))
            return nullptr;
        t->data = c;
    }
    return t;
}

int decompress(std::string input, std::string output)
{
    std::ifstream in(input, std::ios::binary);
    if (!in.is_open())
    {
        return 3;
    }

    uint8_t ext_size = 0;

    in.read(reinterpret_cast<char *>(&ext_size), sizeof(uint8_t));

    char c;
    std::string ext = ".";
    for (size_t i = 0; i < ext_size; i++)
    {
        if (in.get(c))
        {
            ext += c;
        }
        else
        {
            return 5;
        }
    }

    output += ext;

    std::ofstream out(output, std::ios::binary);
    if (!out.is_open())
    {
        return 4;
    }

    uint64_t file_size = 0;

    in.read(
        reinterpret_cast<char *>(&file_size),
        sizeof(file_size));

    node *head = readtree(in);

    node *t = head;

    uint8_t x;
    uint8_t seek = 0;

    for (uint64_t i = 0; i < file_size; i++)
    {

        while (t->left || t->right)
        {
            if (seek == 0)
            {
                seek = 128;
                in.read(reinterpret_cast<char *>(&x), sizeof(x));
            }
            bool val = seek & x;
            seek >>= 1;
            if (val)
                t = t->right;
            else
                t = t->left;
        }
        out << t->data;
        t = head;
    };

    if (ext_size == 0)
    {
        return 6;
    }
    return 0;
}

void show_reason(int status)
{
    if (status == 0)
    {
        std::cout << "OK" << std::endl;
    }
    else
    {
        if (status == 1)
        {
            std::cout << "compressor input file error" << std::endl;
        }
        else if (status == 2)
        {
            std::cout << "compressor output file error" << std::endl;
        }
        else if (status == 3)
        {
            std::cout << "decompressor input file error" << std::endl;
        }
        else if (status == 4)
        {
            std::cout << "decompressor output file error" << std::endl;
        }
        else if (status == 5)
        {
            std::cout << "compressed file was corrupted" << std::endl;
        }
        else if (status == 6)
        {
            std::cout << "file extension could not be recovered" << std::endl;
        }
        else
            std::cout << "Error" << std::endl;
    }
}

void show_help(){
    std::cout << "Usage:" <<std::endl;
    std::cout << "huffcodec compress \"<input_file_path>\" \"<output_file_path>\"" << std::endl;
    std::cout << "huffcodec decompress \"<input_file_path>\" \"<output_file_path>\"" << std::endl;
}

int main(int argc, char* argv[])
{
    
    if(argc<4){
        show_help();
        return 1;
    }

    std::string mode = argv[1];
    std::string input = argv[2];
    std::string output = argv[3];

    int status = 0;
    
    if (mode == "compress")
    {
        status = compress(input,output);
    }else if(mode == "decompress"){
        status = decompress(input,output);
    }else {
        std::cout << "Unknown mode: " << mode << std::endl;
    }
    

    if (status!=0)
    {
        show_reason(status);
    }
    return status;

}
