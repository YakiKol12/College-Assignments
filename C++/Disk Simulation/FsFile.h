#ifndef FSFILE_H
#define FSFILE_H


class FsFile {
    int file_size;
    int block_in_use;
    int index_block;
    int block_size;

public:
    FsFile(int _block_size) {
        file_size = 0;
        block_in_use = 0;
        block_size = _block_size;
        index_block = -1;
    }

    int getIndexBlock() {
        return this->index_block;
    }

    void setIndexBlock(int newIndex) {
        this->index_block = newIndex;
    }

    int getBlocksInUse() {
        return this->block_in_use;
    }

    void setBlockInUse(int num) {
        this->block_in_use = num;
    }

    int getFileSize() {
        return this->file_size;
    }

    void setFileSize(int num) {
        this->file_size += num;
    }

    int getRemainingMemoryInBlock() {
        return block_in_use * block_size - file_size;
    }

};



#endif //FSFILE_H
