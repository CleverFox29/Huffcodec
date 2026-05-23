# Huffcodec
A lossless file compressing and decompressing software that works in the terminal, based on the huffman tree algorithm. 
## Features
1) Lossless: The rebuilt file it exactly same as the original file
2) Corruption detection: corrupted files or tampered files will be caught early.
3) Storage of file extension: extension is preserved and will be appended to the name of the decopressor's output file
4) Speed: native c++ code with negligible overheads.
## Usage
1) huffcodec compress "<input_file_path>" "<output_file_path>"
2) huffcodec decompress "<input_file_path>" "<output_file_path>"
