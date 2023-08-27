#include <stdint.h>

#define FREE(ptr) do { free(ptr); ptr = NULL; } while (0)

// Local File Header (LFH) struct
#pragma pack (push, 1)
typedef struct {

    // Signature of the local file header
    uint32_t signature; // = 0x04034b50 (for little-endian 0x50 0x4b 0x03 0x04)
    // Version needed to extract
    uint16_t version_to_extract;
    // General purpose bit flag
    uint16_t general_purpose_bit_flag;
    // Compression method (0 - no compression, 8 - deflated)
    uint16_t compression_method;
    // File modification time
    uint16_t modification_time;
    // File modification date
    uint16_t modification_date;
    // CRC-32 checksum
    uint32_t crc32;
    // Compressed size
    uint32_t compressed_size;
    // Uncompressed size
    uint32_t uncompressed_size;
    // Length of the file name
    uint16_t filename_length;
    // Length of the extra field
    uint16_t extra_field_length;
    // Name of the file including an optional relative path (with length from 0 to 64KB)
    uint8_t* filename;
    // Extra field to store additional information (with length from 0 to 64KB)
    uint8_t* extra_field;

} LFH_typedef;
#pragma pack (pop)

typedef struct {

    LFH_typedef LFH;
    size_t offset_in_file;
    size_t size;

} LFH_handle_typedef;

// Central Directory File Header (CDFH) struct
#pragma pack (push, 1)
typedef struct {

    // Signature of the file header
    uint32_t signature; // = 0x02014b50 (for little-endian 0x50 0x4b 0x01 0x02)
    // Version made by
    uint16_t version_made_by;
    // Version needed to extract
    uint16_t version_to_extract;
    // General purpose bit flag
    uint16_t general_purpose_bit_flag;
    // Compression method (0 - no compression, 8 - deflated)
    uint16_t compression_method;
    // File modification time
    uint16_t modification_time;
    // File modification date
    uint16_t modification_date;
    // CRC-32 checksum
    uint32_t crc32;
    // Compressed size
    uint32_t compressed_size;
    // Uncompressed size
    uint32_t uncompressed_size;
    // Length of the file name
    uint16_t filename_length;
    // Length of the extra field
    uint16_t extra_field_length;
    // Length of the file comment
    uint16_t file_comment_length;
    // Number of the disk on which this file exists
    uint16_t disk_number;
    // Internal file attributes
    uint16_t internal_file_attributes;
    // External file attributes
    uint32_t external_file_attributes;
    // Offset of where to find the corresponding local file header from the start of the disk
    uint32_t local_file_header_offset;
    // Name of the file including an optional relative path (with length from 0 to 64KB)
    uint8_t* filename;
    // Extra field to store additional information (with length from 0 to 64KB)
    uint8_t* extra_field;
    // Optional comment for the file (with length from 0 to 64KB)
    uint8_t* file_comment;

} CDFH_typedef;
#pragma pack (pop)

typedef struct {

    CDFH_typedef CDFH;
    size_t offset_in_file;
    size_t size;

} CDFH_handle_typedef;

// End Of Central Directory (EOCD) struct
#pragma pack (push, 1)
typedef struct {

    // Signature of end of central directory record
    uint32_t signature; // = 0x06054b50 (for little-endian 0x50 0x4b 0x05 0x06)
    // Number of the disk containing the end of central directory record
    uint16_t disk_number;
    // Number of the disk on which the central directory starts
    uint16_t start_disk_number;
    // Number of central directory entries on this disk
    uint16_t number_of_central_directory_entries_on_disk;
    // Total number of entries in the central directory
    uint16_t number_of_central_directory_entries_total;
    // Size of the entire central directory in bytes
    uint32_t central_directory_size;
    // Offset of the start of the central directory on the disk on which the central directory starts
    // i.e. offset from the archive start
    uint32_t central_directory_offset;
    // Length of the comment
    uint16_t comment_length;
    // Optional comment (with length from 0 to 64KB)
    uint8_t* comment;

} EOCD_typedef;
#pragma pack (pop)

typedef struct {

    EOCD_typedef EOCD;
    size_t offset_in_file;
    size_t size;

} EOCD_handle_typedef;

/**
 * @brief Binary read file and returns an array of bytes and file size.
 * 
 * @param filename Name of a file to read
 * @param size Pointer to a size of the file
 * @return uint8_t* Array of bytes
 */
uint8_t* read_bytes_from_file(char* filename, size_t* size);

/**
 * @brief Search Local File Header (LFH) in the data starting with start_offset_in_data byte.
 * Save LFH in structure if it is found.
 * 
 * @param LFH_handle LFH structure
 * @param data Data where LFH is searched
 * @param size Size of the data
 * @param start_offset_in_data Data byte from which start to search
 * @return uint8_t Flag if search is success or not
 */
uint8_t LFH_get_one(
    LFH_handle_typedef* LFH_handle,
    uint8_t* data,
    size_t size,
    size_t start_offset_in_data
);

/**
 * @brief Search all Central Directory File Headers (CDFH) in the data starting with start_offset_in_data byte.
 * Save each CDFH in structure and all CDFHs in array.
 * 
 * @param CDFH_handles Array of CDFH structures
 * @param CDFH_handles_size CDFH count in the data (know it from EOCD)
 * @param data Data where CDFHs are searched
 * @param start_offset_in_data Data byte from which start to search
 */
void CDFH_get_all(
    CDFH_handle_typedef** CDFH_handles,
    size_t CDFH_handles_size,
    uint8_t* data,
    size_t start_offset_in_data
);

/**
 * @brief Search End Of Central Directory (EOCD) in the data. Save EOCD in structure if it is found.
 * 
 * @param EOCD_handle EOCD structure
 * @param data Data where EOCD is searched
 * @param size Size of the data
 * @return uint8_t Flag if search is success or not
 */
uint8_t EOCD_get(
    EOCD_handle_typedef* EOCD_handle,
    uint8_t* data,
    size_t size
);

/**
 * @brief Print LFH structure.
 * 
 * @param LFH_handle LFH structure
 */
void LFH_print(LFH_handle_typedef* LFH_handle);

/**
 * @brief Print CDFH structures.
 * 
 * @param CDFH_handles Array of CDFH structures
 * @param start_index Index from which start to print CDFHs
 * @param end_index Index up until which to print CDFHs
 */
void CDFH_print_all(CDFH_handle_typedef* CDFH_handles, int start_index, int end_index);

/**
 * @brief Print EOCD structure.
 * 
 * @param EOCD_handle EOCD structure
 */
void EOCD_print(EOCD_handle_typedef* EOCD_handle);
