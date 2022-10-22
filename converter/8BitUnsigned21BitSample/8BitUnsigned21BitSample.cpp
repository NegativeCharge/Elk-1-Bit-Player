#include <stdio.h>
#include <vector>

#define MAX_BUFFER_SIZE 4096 * 1024
typedef unsigned char byte_t;
std::vector<byte_t> buffer;
std::vector<int> output_data;
unsigned int buffersize;					// number of bytes in the buffer

typedef unsigned char byte;
FILE* InputFile;

int main(int argc, char* argv[])
{
    printf("Convert 8-bit unsigned audio to 1-bit sample files - 21-Oct-2022\n\n");

    if (argc < 3) {
        printf("8BitUnsigned21BitSample.exe <filename> <output>\n");
        printf(" <filename> - 8-bit Unsigned audio file to read.\n");
        printf(" <output> - 1-bit sample file to read.\n");

        return -1;
    }

    int arg = 1;
    int readdata = 0;
    char* fn = argv[arg];
    bool hasLooped = false;

    InputFile = fopen(fn, "rb");
    if (InputFile == NULL) {
        printf("File not found.\n");
        return 1;
    }

    arg++;

    printf("\rReading %s : ", fn);

    buffer.resize(MAX_BUFFER_SIZE);
    buffersize = fread(&buffer[0], sizeof(std::vector<byte_t>::value_type), buffer.size(), InputFile);
    printf("%d bytes\n", buffersize);
    fclose(InputFile);

    uint32_t offset = 0;
    uint8_t output_bitrate_divisor = 1;
    uint8_t process_begun = 0;
    uint8_t obsample_bits = 0; 
    uint8_t obsample_bytes = 0;

    for (uint32_t j = 0; j < buffersize; j++) {
        if (buffer[offset] > 128) {
            output_data.push_back(255);
        }
        else {
            output_data.push_back(0);
        }
        int z;
        for (z = 0; z < output_bitrate_divisor; z++) {
            offset++;
            if (offset > buffersize) {
                offset = -1;
                goto end_of_src_samples;
            }
        }
    }

end_of_src_samples:

    // Write out the sample file
    printf("\n\nGoing to write sample file '%s'\n", argv[arg]);
    FILE* fp = fopen(argv[arg], "wb");
    if (NULL == fp) {
        printf("Can't open output file '%s'\n", argv[arg]);
        return 1;
    }

    /* Get 8 bytes from buffer */
    for (uint32_t j = 0; j < buffersize - 8; j += 8) {
        if (process_begun) {
            printf(", ");
        }
        else 
        { 
            process_begun = 1;
        }

        if (!(obsample_bytes % 15)) { 
            printf("\n"); 
        }

        /* Move through those 8 bytes and convert to binary. 0x00 = 0, 0x255 = 1. */
        unsigned char converted_bits;
        converted_bits = 0;

        for (uint8_t k = 0; k < 8; k++) {
            if (k >= buffersize) {
                /* Trying to read past buffer? */
                fprintf(stderr, "Error: Tried to exceed buffersize.\n");
                exit(1);
            }

            converted_bits = converted_bits << 1;
            converted_bits = converted_bits + (output_data[j + k] == 255);
            obsample_bits++;
        }

        printf("0x%02X", converted_bits);
        fputc(converted_bits, fp);
        obsample_bytes++;
    }

    fclose(fp);

    printf("\n\n** DONE **\n");
    return 0;
}