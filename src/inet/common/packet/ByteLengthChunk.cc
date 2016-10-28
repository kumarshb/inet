//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "inet/common/packet/ByteLengthChunk.h"

ByteLengthChunk::ByteLengthChunk(int64_t byteLength) :
    byteLength(byteLength)
{
    assert(byteLength >= 0);
}

void ByteLengthChunk::setByteLength(int64_t byteLength)
{
    assertMutable();
    assert(byteLength >= 0);
    this->byteLength = byteLength;
}

void ByteLengthChunk::replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength)
{
    setByteLength(byteLength);
}

std::shared_ptr<Chunk> ByteLengthChunk::merge(const std::shared_ptr<Chunk>& other) const
{
    if (std::dynamic_pointer_cast<ByteLengthChunk>(other) != nullptr) {
        auto otherByteLengthChunk = std::static_pointer_cast<ByteLengthChunk>(other);
        return std::make_shared<ByteLengthChunk>(byteLength + otherByteLengthChunk->byteLength);
    }
    else
        return nullptr;
}

std::string ByteLengthChunk::str() const
{
    std::ostringstream os;
    os << "ByteLengthChunk, byteLength = " << byteLength;
    return os.str();
}
