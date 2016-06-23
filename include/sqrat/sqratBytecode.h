//
// SqratBytecode: Script bytecode saving and loading
//

//
// Copyright (c) 2009 Brandon Jones
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//  claim that you wrote the original software. If you use this software
//  in a product, an acknowledgment in the product documentation would be
//  appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not be
//  misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//  distribution.
//

#ifndef _SCRAT_BYTECODE_H_
#define _SCRAT_BYTECODE_H_

#include <squirrel.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

namespace Sqrat {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper class for managing Squirrel scripts bytecode
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Bytecode {
public:

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Default constructor
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Bytecode()
        : m_data(0)
        , m_size(0)
        , m_readpos(0)
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Default destructor
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ~Bytecode() {
        if (m_data) {
            free(m_data);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Saves bytecode to file
    ///
    /// \param filename File name to save bytecode to
    /// \returns SQ_OK on success, SQ_ERROR on failure
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    SQRESULT SaveToFile(const char * filename) const {
        if (!m_data || m_size <= 0)
            return SQ_ERROR;
        FILE * ofile = fopen(filename, "wb");
        if (ofile) {
            size_t bytes_written = fwrite(m_data, 1, m_size, ofile);
            fclose(ofile);
            if (bytes_written == m_size) return SQ_OK;
            else return SQ_ERROR;
        }
        return SQ_ERROR;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Loads bytecode from file
    ///
    /// \param filename File name to load bytecode from
    /// \returns SQ_OK on success, SQ_ERROR on failure
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    SQRESULT LoadFromFile(const char * filename) {
        if (m_data) {
            free(m_data);
        }
        FILE * ifile = fopen(filename, "rb");
        if (ifile) {
            fseek(ifile, 0, SEEK_END);
            long fsize = ftell(ifile);
            if (fsize == 0) {
                fclose(ifile);
                return SQ_ERROR;
            }
            fseek(ifile, 0, SEEK_SET);
            m_size = static_cast<size_t>(fsize);
            m_data = static_cast<char *>(malloc(m_size));
            size_t bytes_read = fread(m_data, 1, m_size, ifile);
            fclose(ifile);
            if (bytes_read == m_size) return SQ_OK;
            else return SQ_ERROR;
        }
        return SQ_ERROR;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Return pointer to bytecode
    ///
    /// \returns Pointer to bytecode data
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline void * Data() const {
        return m_data;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Copies bytecode from provided buffer
    ///
    /// \param data Pointer to buffer containing bytecode
    /// \param size Size of buffer containing bytecode
    /// \returns SQ_OK on success, SQ_ERROR on failure
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    SQRESULT SetData(void * data, size_t size) {
        if (m_data) {
            free(m_data);
        }
        m_data = static_cast<char *>(malloc(size));
        memcpy(m_data, data, size);
        m_size = size;
        m_readpos = 0;
        return SQ_OK;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Appends bytecode
    ///
    /// \param data Pointer to buffer containing bytecode to append
    /// \param size Size of buffer containing bytecode to append
    /// \returns Number of bytes appended
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    SQInteger AppendData(void * data, size_t size) {
        if (!m_data) {
            m_data = static_cast<char *>(malloc(size));
        }
        else {
            m_data = static_cast<char *>(realloc(m_data, m_size + size));
        }
        memcpy(m_data + m_size, data, size);
        m_size += size;
        return static_cast<SQInteger>(size);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Reads bytecode
    ///
    /// \param data Pointer to receiving buffer
    /// \param size Number of bytes to read
    /// \returns Number of read bytes or -1 if error occurs or there is no more data available
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    SQInteger ReadData(void * data, size_t size) {
        if (!m_data || m_size == 0 || m_readpos == m_size)
            return -1;
        size_t bytes_to_read = (m_readpos + size <= m_size) ? size : m_size - m_readpos;
        memcpy(data, m_data + m_readpos, bytes_to_read);
        m_readpos += bytes_to_read;
        return static_cast<SQInteger>(bytes_to_read);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Returns bytecode size
    ///
    /// \returns Bytecode size in bytes
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline size_t Size() const {
        return m_size;
    }

private:
    char * m_data;    // Buffer holding bytecode
    size_t m_size;    // Bytecode size
    size_t m_readpos; // Current bytecode ReadData() position
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper bytecode reader callback to use with sq_readclosure
///
/// \param user_data Pointer to \a Bytecode object to read from
/// \param data Pointer to receiving buffer
/// \param size Number of bytes to read
///
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SQInteger BytecodeReader(SQUserPointer user_data, SQUserPointer data, SQInteger size) {
    Bytecode * bytecode = reinterpret_cast<Bytecode *>(user_data);
    return bytecode->ReadData(data, static_cast<size_t>(size));;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper bytecode writer callback to use with sq_writeclosure
///
/// \param user_data Pointer to \a Bytecode object to write to
/// \param data Pointer to bytecode data
/// \param size Number of bytes to write
///
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SQInteger BytecodeWriter(SQUserPointer user_data, SQUserPointer data, SQInteger size) {
    Bytecode * bytecode = reinterpret_cast<Bytecode *>(user_data);
    return bytecode->AppendData(data, static_cast<size_t>(size));
}

}

#endif //_SCRAT_BYTECODE_H_
