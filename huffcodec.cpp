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

// hasfunction for file verification;
uint64_t crc64(std::ifstream &in)
{
    uint64_t crc = 8558055805580558055;
    uint64_t poly = 8558055805580558055;
    char c;
    while (in.get(c))
    {
        uint64_t x = c;
        crc ^= x << ((sizeof(crc) - sizeof(c)) * 8);

        for (size_t i = 0; i < 8; i++)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ poly;
            }
            else
            {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

struct compare
{
    bool operator()(node *a, node *b)
    {
        return a->f > b->f;
    }
};


//bitwise writing mechanism
uint8_t write_cursor = 128;
uint8_t x_write = 0;
void flush(std::ofstream &out)
{
    if (write_cursor == 128)
        return;
    out.write(
        reinterpret_cast<char *>(&x_write),
        sizeof(x_write));
    x_write = 0;
    write_cursor = 128;
}
void collectNWrite(bool val, std::ofstream &out)
{
    x_write += write_cursor * val;
    write_cursor >>= 1;
    if (write_cursor == 0)
    {
        flush(out);
    }
}


//bitwise read mechanism
uint8_t cursor = 0;
uint8_t x = 0;
void next_byte(std::ifstream & in){
    cursor = 128;
    in.read(reinterpret_cast<char*>(&x), sizeof(x));
}
bool collectNRead(std::ifstream& in){
    if (cursor == 0)
    {
        next_byte(in);
    }
    bool val = cursor & x;
    cursor>>=1;
    return val;
}

void map_path(node *head, std::map<char, std::string> &huffmap, std::ofstream &out, std::string path = "", bool root = true)
{
    if (!head)
        return;

    if (!head->left && !head->right)
    {
        collectNWrite(true,out);
        char c = head->data;
        for(int i = 0; i < sizeof(c)*8;i++){
            collectNWrite(1<<(sizeof(c)*8-i-1) & c,out);
        }
        huffmap[head->data] = path;
    }
    else
    {
        collectNWrite(false,out);
    }
    map_path(head->left, huffmap, out, path + "0",false);
    map_path(head->right, huffmap, out, path + "1",false);
    
}

// unused method that i thought was useful and later found useless
void bunch_duplicates_frequencies(std::priority_queue<node *, std::vector<node *>, compare> &pq)
{
    std::priority_queue<node *, std::vector<node *>, compare> temp;
    bool dup = true;
    while (dup)
    {
        dup = false;
        while (!pq.empty())
        {

            node *i = pq.top();
            pq.pop();
            std::vector<node *> v;
            v.push_back(i);
            while (!pq.empty() && pq.top()->f == i->f)
            {
                v.push_back(pq.top());
                pq.pop();
                dup = true;
            }
            int n = v.size();

            std::vector<node *> tree;
            tree.reserve(2 * n - 1);
            tree.assign(n - 1, nullptr);
            tree.insert(tree.end(), v.begin(), v.end());

            for (size_t i = n - 1; i-- > 0;)
            {
                node *&t = tree[i];
                t = new node();
                t->left = tree[2 * i + 1];
                t->right = tree[2 * i + 2];
                t->f = t->left->f + t->right->f;
            }
            temp.push(tree[0]);
        }
        std::swap(pq, temp);
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

    uint64_t crc; // placeholder is written to allocate empty space to be written on later
    out.write(reinterpret_cast<char *>(&crc), sizeof(crc));

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


    //reading file extension
    std::string extension = "";
    for (size_t i = input.size(); i-- > 0;)
    {
        if (input[i] == '.')
        {
            break;
        }
        else if (input[i] == '\\' || input[i] == '/')
        {
            extension = "";
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

    //writing the extension
    out.write(reinterpret_cast<char *>(&ext_size), sizeof(ext_size));
    for (int i = 0; i < ext_size; i++)
    {
        out << extension[i];
    }

    
    //writing file size in bytes
    uint64_t file_size = head->f;
    out.write(reinterpret_cast<char *>(&file_size), sizeof(file_size));

    // for quick encoding
    std::map<char, std::string> huffmap;
    // populates huffmap also writes map to file in preorder
    map_path(head, huffmap, out);

    in.clear();
    in.seekg(0);

    // writes the compressed file
    while (in.get(c))
    {
        for (char i : huffmap[c])
        {
            if (i == '1')
            {
                collectNWrite(true, out);
            }
            else if (i == '0')
            {
                collectNWrite(false, out);
            }
        }
    }
    flush(out);
    out.flush();
    out.clear();

    // calculates and writes the hash to the beginning of the file
    out.seekp(0);
    std::ifstream outreader(output, std::ios::binary); //object to read the file to make the hash
    outreader.seekg(sizeof(crc));
    crc = crc64(outreader);
    out.write(reinterpret_cast<char *>(&crc), sizeof(crc));

    return 0;
}

node *readtree(std::ifstream &in)
{
    node *t = new node();
    bool val = collectNRead(in);
    if (val)
    {
        char c = 0;
        for (int i = 0; i < sizeof(c)*8; i++)
        {
            c+= collectNRead(in) * (1<<(sizeof(c)*8-i-1));
        }
        t->data = c;
    }
    else
    {
        t->left = readtree(in);
        t->right = readtree(in);
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

    //hash verification
    uint64_t crc;
    in.read(reinterpret_cast<char *>(&crc), sizeof(crc));
    if (crc != crc64(in))
    {
        return 7;
    }
    in.clear();
    in.seekg(sizeof(crc));

    //reading extension from file
    uint8_t ext_size = 0;
    in.read(reinterpret_cast<char *>(&ext_size), sizeof(uint8_t));
    char c;
    std::string ext = "";
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


    //reading outputfile extension
    std::string output_ext = "";
    for (size_t i = output.size(); i-- > 0;)
    {
        if (output[i] == '.')
        {
            break;
        }
        else if (output[i] == '\\' || output[i] == '/')
        {
            output_ext = "";
            break;
        }
        else
        {
            output_ext = output[i] + output_ext;
        }
    }
    if (output == output_ext)
    {
        output_ext = "";
    }
    
    //if both extensions are not same then append the new extension
    if (ext != output_ext)
    {
        output += "." + ext;
    }

    std::ofstream out(output, std::ios::binary);
    if (!out.is_open())
    {
        return 4;
    }

    //read file size
    uint64_t file_size = 0;
    in.read(
        reinterpret_cast<char *>(&file_size),
        sizeof(file_size));

    
    // reconstruction of tree
    node *head = readtree(in);
    node *t = head;

    //interpreting the file
    uint8_t x;
    uint8_t seek = 0;
    for (uint64_t i = 0; i < file_size; i++)
    {

        while (t->left || t->right)
        {
            bool val = collectNRead(in);
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
        else if (status == 7)
        {
            std::cout << "file corruption detected" << std::endl;
        }

        else
            std::cout << "Error" << std::endl;
    }
}

void show_help()
{
    std::cout << "Usage:" << std::endl;
    std::cout << "huffcodec compress \"<input_file_path>\" \"<output_file_path>\"" << std::endl;
    std::cout << "huffcodec decompress \"<input_file_path>\" \"<output_file_path>\"" << std::endl;
}

int main(int argc, char *argv[])
{

    if (argc < 4)
    {
        show_help();
        return 1;
    }

    std::string mode = argv[1];
    std::string input = argv[2];
    std::string output = argv[3];

    int status = 0;

    if (mode == "compress")
    {
        status = compress(input, output);
    }
    else if (mode == "decompress")
    {
        status = decompress(input, output);
    }
    else
    {
        std::cout << "Unknown mode: " << mode << std::endl;
    }

    if (status != 0)
    {
        show_reason(status);
    }
    return status;
}
