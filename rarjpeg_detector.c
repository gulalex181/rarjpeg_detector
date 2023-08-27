#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "rarjpeg_detector.h"

int main(int argc, char* argv[]) {

    if (argc != 2) {
        printf("File to test is required. Provide filepath as an agrument.");
        return EXIT_FAILURE;
    }

    char* filename = argv[1];

    LFH_handle_typedef file_LFH_handle_first;
    CDFH_handle_typedef* file_CDFH_handles;
    EOCD_handle_typedef file_EOCD_handle;

    size_t file_size;
    uint8_t* file_data;

    // Read bytes from the file-candidate
    if ((file_data = read_bytes_from_file(filename, &file_size)) == NULL) {
        return EXIT_FAILURE;
    }

    // Search of End Of Central Directory
    if (EOCD_get(&file_EOCD_handle, file_data, file_size)) {

        // There is a Zip archive in the file

        // EOCD_print(&file_EOCD_handle);

        CDFH_get_all(
            &file_CDFH_handles,
            file_EOCD_handle.EOCD.number_of_central_directory_entries_total,
            file_data,
            file_size - file_EOCD_handle.size - file_EOCD_handle.EOCD.central_directory_size
        );

        LFH_get_one(
            &file_LFH_handle_first,
            file_data,
            file_size,
            0
        );

        // LFH_print(&file_LFH_handle_first);

        // printf("File size: %d\n", file_size);
        // printf("EOCD size: %d\n", file_EOCD_handle.size);
        // printf("CDFH count: %d\n\n", file_EOCD_handle.EOCD.number_of_central_directory_entries_total);

        // CDFH_print_all(file_CDFH_handles, 0, file_EOCD_handle.EOCD.number_of_central_directory_entries_total - 1);
        // CDFH_print_all(file_CDFH_handles, 0, 0);

        // Determine if the file is a rarjpeg or a regular archive
        if (file_LFH_handle_first.offset_in_file == 0) {
            printf("File %s is a regular archive\n", filename);
        } else {
            printf("File %s is a rarjpeg\n", filename);
        }

        printf("Files in the archive (%d files):\n", file_EOCD_handle.EOCD.number_of_central_directory_entries_total);
        
        for (int i = 0; i < file_EOCD_handle.EOCD.number_of_central_directory_entries_total; i++) {
            printf("%s\n", file_CDFH_handles[i].CDFH.filename);
        }

    } else {

        // There is no Zip archive in the file
    
        printf("There is no Zip archive in the file %s\n", filename);

    }

    // Free the allocated memory
    // for (int i = 0; i < file_EOCD_handle.EOCD.number_of_central_directory_entries_total; i++) {

    //     if (file_CDFH_handles[i].CDFH.filename) FREE(file_CDFH_handles[i].CDFH.filename);
    //     if (file_CDFH_handles[i].CDFH.extra_field) FREE(file_CDFH_handles[i].CDFH.extra_field);
    //     if (file_CDFH_handles[i].CDFH.file_comment) FREE(file_CDFH_handles[i].CDFH.file_comment);

    // }

    if (file_data) FREE(file_data);
    if (file_CDFH_handles) FREE(file_CDFH_handles);
    // if (file_EOCD_handle.EOCD.comment) FREE(file_EOCD_handle.EOCD.comment);
    // if (file_LFH_handle_first.LFH.filename) FREE(file_LFH_handle_first.LFH.filename);
    // if (file_LFH_handle_first.LFH.extra_field) FREE(file_LFH_handle_first.LFH.extra_field);

    return EXIT_SUCCESS;

}

uint8_t* read_bytes_from_file(char* filename, size_t* size) {

    FILE *file;

    // Try to open the file
    if ((file = fopen(filename, "rb")) == NULL) {
        printf("Cannot open file %s\n", filename);
        return NULL;
    }

    // Get size of the file
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the file
    // uint8_t* data = malloc(*size);
    // memset(data, 0, *size);
    uint8_t* data = (uint8_t*)calloc(*size, sizeof(uint8_t));

    // Read the file's bytes
    // (read 1 byte `size` times)
    if (fread(data, 1, *size, file) != *size) {
        printf("Cannot read file %s entirely\n", filename);
        return NULL;
    }

    // Close the file
    if (fclose(file) == EOF) {
        printf("Cannot close file %s\n", filename);
    }

    return data;

}

uint8_t LFH_get_one(
    LFH_handle_typedef* LFH_handle,
    uint8_t* data,
    size_t size,
    size_t start_offset_in_data
) {

    uint8_t is_LFH_signature_found = 0;
    uint8_t LFH_structure_fixed_size = 30; // bytes

    if (size < LFH_structure_fixed_size) return is_LFH_signature_found;

    for (size_t i = 0; i < size - 3; i++) {

        if (*((char*)(data + start_offset_in_data + i + 0)) == 0x50
         && *((char*)(data + start_offset_in_data + i + 1)) == 0x4b
         && *((char*)(data + start_offset_in_data + i + 2)) == 0x03
         && *((char*)(data + start_offset_in_data + i + 3)) == 0x04) {

            is_LFH_signature_found = 1;
            LFH_handle->offset_in_file = start_offset_in_data + i;

            memcpy(&LFH_handle->LFH, data + LFH_handle->offset_in_file, LFH_structure_fixed_size);

            // Allocate memory for LFH's filename
            // 1 extra byte is for null-terminator
            LFH_handle->LFH.filename
                = (uint8_t*)malloc((LFH_handle->LFH.filename_length + 1) * sizeof(uint8_t));

            // Add comment to the LFH structure
            memcpy(
                LFH_handle->LFH.filename,
                data + LFH_handle->offset_in_file + LFH_structure_fixed_size,
                LFH_handle->LFH.filename_length
            );
            // Add null-terminator to the comment
            LFH_handle->LFH.filename[LFH_handle->LFH.filename_length] = '\0';

            // Allocate memory for LFH's extra field
            LFH_handle->LFH.extra_field
                = (uint8_t*)malloc(LFH_handle->LFH.extra_field_length * sizeof(uint8_t));

            // Add extra field to the LFH structure
            memcpy(
                LFH_handle->LFH.extra_field,
                data + LFH_handle->offset_in_file + LFH_structure_fixed_size + LFH_handle->LFH.filename_length,
                LFH_handle->LFH.extra_field_length
            );

            LFH_handle->size
                = LFH_structure_fixed_size + LFH_handle->LFH.filename_length + LFH_handle->LFH.extra_field_length;

            break;

        }

    }

    return is_LFH_signature_found;

}

void CDFH_get_all(
    CDFH_handle_typedef** CDFH_handles,
    size_t CDFH_handles_size,
    uint8_t* data,
    size_t start_offset_in_data
) {

    uint8_t  CDFH_structure_fixed_size = 46; // bytes
    uint8_t* current_pointer_in_CD = data + start_offset_in_data;
    size_t current_offset_in_data = start_offset_in_data;

    // Allocate memory for CDFHs
    *CDFH_handles = (CDFH_handle_typedef*)malloc(CDFH_handles_size * sizeof(CDFH_handle_typedef));

    for (size_t CDFH_handle_number = 0; CDFH_handle_number < CDFH_handles_size; CDFH_handle_number++) {

        // Save CDFH offset
        (*CDFH_handles)[CDFH_handle_number].offset_in_file = current_offset_in_data;

        // Fill the CDFH structure with fixed size data
        memcpy(&(*CDFH_handles)[CDFH_handle_number].CDFH, current_pointer_in_CD, CDFH_structure_fixed_size);

        // Shift the pointer to the end of the CDFH structure's fixed size data
        current_pointer_in_CD += CDFH_structure_fixed_size;

        // Save current offset
        current_offset_in_data += CDFH_structure_fixed_size;

        // If there is a non-zero filename
        if ((*CDFH_handles)[CDFH_handle_number].CDFH.filename_length > 0) {

            // Allocate memory for CDFH's filename
            // 1 extra byte is for null-terminator
            (*CDFH_handles)[CDFH_handle_number].CDFH.filename
                = (uint8_t*)malloc(((*CDFH_handles)[CDFH_handle_number].CDFH.filename_length + 1) * sizeof(uint8_t));

            // Add filename to the CDFH structure
            memcpy(
                (*CDFH_handles)[CDFH_handle_number].CDFH.filename,
                current_pointer_in_CD,
                (*CDFH_handles)[CDFH_handle_number].CDFH.filename_length
            );
            // Add null-terminator to the filename
            (*CDFH_handles)[CDFH_handle_number].CDFH.filename[(*CDFH_handles)[CDFH_handle_number].CDFH.filename_length] = '\0';

            // Shift the pointer to the end of the CDFH structure's filename
            current_pointer_in_CD += (*CDFH_handles)[CDFH_handle_number].CDFH.filename_length;

            // Save current offset
            current_offset_in_data += (*CDFH_handles)[CDFH_handle_number].CDFH.filename_length;

        }

        // If there is a non-zero extra field
        if ((*CDFH_handles)[CDFH_handle_number].CDFH.extra_field_length > 0) {

            // Allocate memory for CDFH's extra field
            (*CDFH_handles)[CDFH_handle_number].CDFH.extra_field
                = (uint8_t*)malloc((*CDFH_handles)[CDFH_handle_number].CDFH.extra_field_length * sizeof(uint8_t));

            // Add extra field to the CDFH structure
            memcpy(
                (*CDFH_handles)[CDFH_handle_number].CDFH.extra_field,
                current_pointer_in_CD,
                (*CDFH_handles)[CDFH_handle_number].CDFH.extra_field_length
            );

            // Shift the pointer to the end of the CDFH structure's extra field
            current_pointer_in_CD += (*CDFH_handles)[CDFH_handle_number].CDFH.extra_field_length;

            // Save current offset
            current_offset_in_data += (*CDFH_handles)[CDFH_handle_number].CDFH.extra_field_length;

        }

        // If there is a non-zero file comment
        if ((*CDFH_handles)[CDFH_handle_number].CDFH.file_comment_length > 0) {

            // Allocate memory for CDFH's file comment
            (*CDFH_handles)[CDFH_handle_number].CDFH.file_comment
                = (uint8_t*)malloc(((*CDFH_handles)[CDFH_handle_number].CDFH.file_comment_length + 1) * sizeof(uint8_t));

            // Add file comment to the CDFH structure
            memcpy(
                (*CDFH_handles)[CDFH_handle_number].CDFH.file_comment,
                current_pointer_in_CD,
                (*CDFH_handles)[CDFH_handle_number].CDFH.file_comment_length
            );
            // Add null-terminator to the file comment
            (*CDFH_handles)[CDFH_handle_number].CDFH.file_comment[(*CDFH_handles)[CDFH_handle_number].CDFH.file_comment_length] = '\0';

            // Shift the pointer to the end of the CDFH structure
            current_pointer_in_CD += (*CDFH_handles)[CDFH_handle_number].CDFH.file_comment_length;

            // Save current offset
            current_offset_in_data += (*CDFH_handles)[CDFH_handle_number].CDFH.file_comment_length;

        }
        
        // Save size of the CDFH
        (*CDFH_handles)[CDFH_handle_number].size = current_offset_in_data - (*CDFH_handles)[CDFH_handle_number].offset_in_file;

    }

}

uint8_t EOCD_get(EOCD_handle_typedef* EOCD_handle, uint8_t* data, size_t size) {

    uint8_t is_EOCD_signature_found = 0;
    uint8_t EOCD_structure_fixed_size = 22; // bytes

    if (size < EOCD_structure_fixed_size) return is_EOCD_signature_found;

    for (size_t i = 0; i < size - 3; i++) {

        if (*((char*)(data + size - i - 0)) == 0x06
         && *((char*)(data + size - i - 1)) == 0x05
         && *((char*)(data + size - i - 2)) == 0x4b
         && *((char*)(data + size - i - 3)) == 0x50) {

            is_EOCD_signature_found = 1;
            EOCD_handle->offset_in_file = size - i - 3;
            EOCD_handle->size = i + 3;

            memcpy(&EOCD_handle->EOCD, data + EOCD_handle->offset_in_file, EOCD_structure_fixed_size);
            
            // Allocate memory for EOCD's comment
            // 1 extra byte is for null-terminator
            EOCD_handle->EOCD.comment
                = (uint8_t*)malloc((EOCD_handle->EOCD.comment_length + 1) * sizeof(uint8_t));

            // Add comment to the EOCD structure
            memcpy(
                EOCD_handle->EOCD.comment,
                data + EOCD_handle->offset_in_file + EOCD_structure_fixed_size,
                EOCD_handle->EOCD.comment_length
            );
            // Add null-terminator to the comment
            EOCD_handle->EOCD.comment[EOCD_handle->EOCD.comment_length] = '\0';

            break;

        }

    }

    return is_EOCD_signature_found;

}

void LFH_print(LFH_handle_typedef* LFH_handle) {

    printf("LFH size: %zu bytes\n", LFH_handle->size);
    printf("LFH offset in file: %zx (%zu) bytes\n", LFH_handle->offset_in_file, LFH_handle->offset_in_file);
    printf("LFH signature (4 bytes): 0x%08x\n", LFH_handle->LFH.signature);

    printf(
        "LFH version to extract (2 bytes): 0x%04x (%d)\n",
        LFH_handle->LFH.version_to_extract,
        LFH_handle->LFH.version_to_extract
    );

    printf(
        "LFH general purpose bit flag (2 bytes): 0x%04x (%d)\n",
        LFH_handle->LFH.general_purpose_bit_flag,
        LFH_handle->LFH.general_purpose_bit_flag
    );

    printf(
        "LFH compression method (2 bytes): 0x%04x (%d)\n",
        LFH_handle->LFH.compression_method,
        LFH_handle->LFH.compression_method
    );

    printf(
        "LFH modification time (2 bytes): 0x%04x (%d)\n",
        LFH_handle->LFH.modification_time,
        LFH_handle->LFH.modification_time
    );

    printf(
        "LFH modification date (2 bytes): 0x%04x (%d)\n",
        LFH_handle->LFH.modification_date,
        LFH_handle->LFH.modification_date
    );

    printf(
        "LFH crc32 (4 bytes): 0x%08x (%d)\n",
        LFH_handle->LFH.crc32,
        LFH_handle->LFH.crc32
    );

    printf(
        "LFH compressed size (4 bytes): 0x%08x (%d)\n",
        LFH_handle->LFH.compressed_size,
        LFH_handle->LFH.compressed_size
    );

    printf(
        "LFH uncompressed size (4 bytes): 0x%08x (%d)\n",
        LFH_handle->LFH.uncompressed_size,
        LFH_handle->LFH.uncompressed_size
    );

    printf(
        "LFH filename length (2 bytes): 0x%04x (%d)\n",
        LFH_handle->LFH.filename_length,
        LFH_handle->LFH.filename_length
    );

    printf(
        "LFH extra field length (2 bytes): 0x%04x (%d)\n",
        LFH_handle->LFH.extra_field_length,
        LFH_handle->LFH.extra_field_length
    );

    if (LFH_handle->LFH.filename_length > 0) {

        printf(
            "LFH filename (%d bytes): %s\n",
            LFH_handle->LFH.filename_length,
            LFH_handle->LFH.filename
        );

    } else {

        printf("LFH filename (0 bytes)\n");

    }

    if (LFH_handle->LFH.extra_field_length > 0) {

        printf("LFH extra field (%d bytes): 0x", LFH_handle->LFH.extra_field_length);

        for (int i = LFH_handle->LFH.extra_field_length - 1; i >= 0; i--) {
            printf("%02x", LFH_handle->LFH.extra_field[i]);
        }

        printf("\n");

    } else {

        printf("LFH extra field (0 bytes)\n");

    }

    printf("\n");

}

void CDFH_print_all(CDFH_handle_typedef* CDFH_handles, int start_index, int end_index) {

    for (int CDFH_handle_number = start_index; CDFH_handle_number <= end_index; CDFH_handle_number++) {

        printf("CDFH index: %d\n", CDFH_handle_number);
        printf("---\n");
        printf("CDFH size: %zu bytes\n", CDFH_handles[CDFH_handle_number].size);

        printf(
            "CDFH offset in file: %zx (%zu) bytes\n",
            CDFH_handles[CDFH_handle_number].offset_in_file,
            CDFH_handles[CDFH_handle_number].offset_in_file
        );
        
        printf("CDFH signature (4 bytes): 0x%08x\n", CDFH_handles[CDFH_handle_number].CDFH.signature);

        printf(
            "CDFH version made by (2 bytes): 0x%04x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.version_made_by,
            CDFH_handles[CDFH_handle_number].CDFH.version_made_by
        );

        printf(
            "CDFH version to extract (2 bytes): 0x%04x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.version_to_extract,
            CDFH_handles[CDFH_handle_number].CDFH.version_to_extract
        );

        printf(
            "CDFH general purpose bit flag (2 bytes): 0x%04x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.general_purpose_bit_flag,
            CDFH_handles[CDFH_handle_number].CDFH.general_purpose_bit_flag
        );

        printf(
            "CDFH compression method (2 bytes): 0x%04x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.compression_method,
            CDFH_handles[CDFH_handle_number].CDFH.compression_method
        );

        printf(
            "CDFH modification time (2 bytes): 0x%04x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.modification_time,
            CDFH_handles[CDFH_handle_number].CDFH.modification_time
        );

        printf(
            "CDFH modification date (2 bytes): 0x%04x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.modification_date,
            CDFH_handles[CDFH_handle_number].CDFH.modification_date
        );

        printf(
            "CDFH crc32 (4 bytes): 0x%08x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.crc32,
            CDFH_handles[CDFH_handle_number].CDFH.crc32
        );

        printf(
            "CDFH compressed size (4 bytes): 0x%08x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.compressed_size,
            CDFH_handles[CDFH_handle_number].CDFH.compressed_size
        );

        printf(
            "CDFH uncompressed size (4 bytes): 0x%08x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.uncompressed_size,
            CDFH_handles[CDFH_handle_number].CDFH.uncompressed_size
        );

        printf(
            "CDFH filename length (2 bytes): 0x%04x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.filename_length,
            CDFH_handles[CDFH_handle_number].CDFH.filename_length
        );

        printf(
            "CDFH extra field length (2 bytes): 0x%04x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.extra_field_length,
            CDFH_handles[CDFH_handle_number].CDFH.extra_field_length
        );

        printf(
            "CDFH file comment length (2 bytes): 0x%04x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.file_comment_length,
            CDFH_handles[CDFH_handle_number].CDFH.file_comment_length
        );

        printf(
            "CDFH disk number (2 bytes): 0x%04x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.disk_number,
            CDFH_handles[CDFH_handle_number].CDFH.disk_number
        );

        printf(
            "CDFH internal file attributes (2 bytes): 0x%04x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.internal_file_attributes,
            CDFH_handles[CDFH_handle_number].CDFH.internal_file_attributes
        );

        printf(
            "CDFH external file attributes (4 bytes): 0x%08x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.external_file_attributes,
            CDFH_handles[CDFH_handle_number].CDFH.external_file_attributes
        );

        printf(
            "CDFH local file header offset (4 bytes): 0x%08x (%d)\n",
            CDFH_handles[CDFH_handle_number].CDFH.local_file_header_offset,
            CDFH_handles[CDFH_handle_number].CDFH.local_file_header_offset
        );

        if (CDFH_handles[CDFH_handle_number].CDFH.filename_length > 0) {

            printf(
                "CDFH filename (%d bytes): %s\n",
                CDFH_handles[CDFH_handle_number].CDFH.filename_length,
                CDFH_handles[CDFH_handle_number].CDFH.filename
            );

        } else {

            printf("CDFH filename (0 bytes)\n");

        }

        if (CDFH_handles[CDFH_handle_number].CDFH.extra_field_length > 0) {

            printf("CDFH extra field (%d bytes): 0x", CDFH_handles[CDFH_handle_number].CDFH.extra_field_length);

            for (int i = CDFH_handles[CDFH_handle_number].CDFH.extra_field_length - 1; i >= 0; i--) {
                printf("%02x", CDFH_handles[CDFH_handle_number].CDFH.extra_field[i]);
            }

            printf("\n");

        } else {

            printf("CDFH extra field (0 bytes)\n");

        }

        if (CDFH_handles[CDFH_handle_number].CDFH.file_comment_length > 0) {

            printf(
                "CDFH file comment (%d bytes): %s\n",
                CDFH_handles[CDFH_handle_number].CDFH.file_comment_length,
                CDFH_handles[CDFH_handle_number].CDFH.file_comment
            );

        } else {

            printf("CDFH file comment (0 bytes)\n");

        }

        printf("\n");

    }

}

void EOCD_print(EOCD_handle_typedef* EOCD_handle) {

    printf("EOCD size: %zu bytes\n", EOCD_handle->size);
    printf("EOCD offset in file: %zx (%zu) bytes\n", EOCD_handle->offset_in_file, EOCD_handle->offset_in_file);
    printf("EOCD signature (4 bytes): 0x%08x\n", EOCD_handle->EOCD.signature);
    
    printf(
        "EOCD disk number (2 bytes): 0x%04x (%d)\n",
        EOCD_handle->EOCD.disk_number,
        EOCD_handle->EOCD.disk_number
    );

    printf(
        "EOCD start disk number (2 bytes): 0x%04x (%d)\n",
        EOCD_handle->EOCD.start_disk_number,
        EOCD_handle->EOCD.start_disk_number
    );

    printf(
        "EOCD number of CD entries on disk (2 bytes): 0x%04x (%d)\n",
        EOCD_handle->EOCD.number_of_central_directory_entries_on_disk,
        EOCD_handle->EOCD.number_of_central_directory_entries_on_disk
    );

    printf(
        "EOCD number of CD entries total (2 bytes): 0x%04x (%d)\n",
        EOCD_handle->EOCD.number_of_central_directory_entries_total,
        EOCD_handle->EOCD.number_of_central_directory_entries_total
    );

    printf(
        "EOCD CD size (4 bytes): 0x%08x (%d)\n",
        EOCD_handle->EOCD.central_directory_size,
        EOCD_handle->EOCD.central_directory_size
    );

    printf(
        "EOCD CD offset (4 bytes): 0x%08x (%d)\n",
        EOCD_handle->EOCD.central_directory_offset,
        EOCD_handle->EOCD.central_directory_offset
    );

    printf(
        "EOCD comment length (2 bytes): 0x%04x (%d)\n",
        EOCD_handle->EOCD.comment_length,
        EOCD_handle->EOCD.comment_length
    );

    if (EOCD_handle->EOCD.comment_length > 0) {

        printf(
            "EOCD comment (%d bytes): %s\n",
            EOCD_handle->EOCD.comment_length,
            EOCD_handle->EOCD.comment
        );

    } else {

        printf("EOCD comment (0 bytes)\n");

    }
    
    printf("\n");

}