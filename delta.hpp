#ifndef NALL_BEAT_DELTA_HPP
#define NALL_BEAT_DELTA_HPP

#include <nall/file.hpp>
#include <nall/filemap.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>

namespace nall {

struct bpsdelta {
  inline void source(const uint8_t* data, unsigned size);
  inline void target(const uint8_t* data, unsigned size);

  inline bool source(const string& filename);
  inline bool target(const string& filename);
  inline bool create(const string& filename, const string& metadata = "");

protected:
  enum : unsigned { SourceRead, TargetRead, SourceCopy, TargetCopy };
  enum : unsigned { Granularity = 1 };

  struct Node {
    unsigned offset = 0;
    Node* next = nullptr;
    Node() = default;
    ~Node() { if(next) delete next; }
  };

  filemap sourceFile;
  const uint8_t* sourceData;
  unsigned sourceSize;

  filemap targetFile;
  const uint8_t* targetData;
  unsigned targetSize;
};

void bpsdelta::source(const uint8_t* data, unsigned size) {
  sourceData = data;
  sourceSize = size;
}

void bpsdelta::target(const uint8_t* data, unsigned size) {
  targetData = data;
  targetSize = size;
}

bool bpsdelta::source(const string& filename) {
  if(sourceFile.open(filename, filemap::mode::read) == false) return false;
  source(sourceFile.data(), sourceFile.size());
  return true;
}

bool bpsdelta::target(const string& filename) {
  if(targetFile.open(filename, filemap::mode::read) == false) return false;
  target(targetFile.data(), targetFile.size());
  return true;
}

bool bpsdelta::create(const string& filename, const string& metadata) {
  file modifyFile;
  if(modifyFile.open(filename, file::mode::write) == false) return false;

  Hash::CRC32 sourceChecksum, modifyChecksum;
  unsigned sourceRelativeOffset = 0, targetRelativeOffset = 0, outputOffset = 0;

  auto write = [&](uint8_t data) {
    modifyFile.write(data);
    modifyChecksum.data(data);
  };

  auto encode = [&](uint64_t data) {
    while(true) {
      uint64_t x = data & 0x7f;
      data >>= 7;
      if(data == 0) {
        write(0x80 | x);
        break;
      }
      write(x);
      data--;
    }
  };

  write('B');
  write('P');
  write('S');
  write('1');

  encode(sourceSize);
  encode(targetSize);

  unsigned markupSize = metadata.length();
  encode(markupSize);
  for(unsigned n = 0; n < markupSize; n++) write(metadata[n]);

  Node* sourceTree[65536];
  Node* targetTree[65536];
  for(unsigned n = 0; n < 65536; n++) sourceTree[n] = nullptr, targetTree[n] = nullptr;

  //source tree creation
  for(unsigned offset = 0; offset < sourceSize; offset++) {
    uint16_t symbol = sourceData[offset + 0];
    sourceChecksum.data(symbol);
    if(offset < sourceSize - 1) symbol |= sourceData[offset + 1] << 8;
    Node *node = new Node;
    node->offset = offset;
    node->next = sourceTree[symbol];
    sourceTree[symbol] = node;
  }

  unsigned targetReadLength = 0;

  auto targetReadFlush = [&]() {
    if(targetReadLength) {
      encode(TargetRead | ((targetReadLength - 1) << 2));
      unsigned offset = outputOffset - targetReadLength;
      while(targetReadLength) write(targetData[offset++]), targetReadLength--;
    }
  };

  while(outputOffset < targetSize) {
    unsigned maxLength = 0, maxOffset = 0, mode = TargetRead;

    uint16_t symbol = targetData[outputOffset + 0];
    if(outputOffset < targetSize - 1) symbol |= targetData[outputOffset + 1] << 8;

    { //source read
      unsigned length = 0, offset = outputOffset;
      while(offset < sourceSize && offset < targetSize && sourceData[offset] == targetData[offset]) {
        length++;
        offset++;
      }
      if(length > maxLength) maxLength = length, mode = SourceRead;
    }

    { //source copy
      Node* node = sourceTree[symbol];
      while(node) {
        unsigned length = 0, x = node->offset, y = outputOffset;
        while(x < sourceSize && y < targetSize && sourceData[x++] == targetData[y++]) length++;
        if(length > maxLength) maxLength = length, maxOffset = node->offset, mode = SourceCopy;
        node = node->next;
      }
    }

    { //target copy
      Node* node = targetTree[symbol];
      while(node) {
        unsigned length = 0, x = node->offset, y = outputOffset;
        while(y < targetSize && targetData[x++] == targetData[y++]) length++;
        if(length > maxLength) maxLength = length, maxOffset = node->offset, mode = TargetCopy;
        node = node->next;
      }

      //target tree append
      node = new Node;
      node->offset = outputOffset;
      node->next = targetTree[symbol];
      targetTree[symbol] = node;
    }

    { //target read
      if(maxLength < 4) {
        maxLength = min((unsigned)Granularity, targetSize - outputOffset);
        mode = TargetRead;
      }
    }

    if(mode != TargetRead) targetReadFlush();

    switch(mode) {
    case SourceRead:
      encode(SourceRead | ((maxLength - 1) << 2));
      break;
    case TargetRead:
      //delay write to group sequential TargetRead commands into one
      targetReadLength += maxLength;
      break;
    case SourceCopy:
    case TargetCopy:
      encode(mode | ((maxLength - 1) << 2));
      signed relativeOffset;
      if(mode == SourceCopy) {
        relativeOffset = maxOffset - sourceRelativeOffset;
        sourceRelativeOffset = maxOffset + maxLength;
      } else {
        relativeOffset = maxOffset - targetRelativeOffset;
        targetRelativeOffset = maxOffset + maxLength;
      }
      encode((relativeOffset < 0) | (abs(relativeOffset) << 1));
      break;
    }

    outputOffset += maxLength;
  }

  targetReadFlush();

  for(unsigned n = 0; n < 32; n += 8) write(sourceChecksum.value() >> n);
  uint32_t targetChecksum = Hash::CRC32(targetData, targetSize).value();
  for(unsigned n = 0; n < 32; n += 8) write(targetChecksum >> n);
  uint32_t outputChecksum = modifyChecksum.value();
  for(unsigned n = 0; n < 32; n += 8) write(outputChecksum >> n);

  modifyFile.close();
  return true;
}

}

#endif
