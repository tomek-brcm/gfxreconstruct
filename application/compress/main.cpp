#include <cassert>

#include "format/file_processor.h"
#include "format/decompress_decoder.h"
#include "util/compressor.h"

int main(int argc, char** argv)
{
    brimstone::format::FileProcessor file_processor;
    std::string                      input_bin_file_name    = "D:\\temp\\brimstone_test.bin";
    std::string                      output_bin_file_name   = "D:\\temp\\brimstone_test_out.bin";
    brimstone::util::CompressionType compression_type       = brimstone::util::kNone;
    std::string                      dst_compression_string = "NONE";
    bool                             print_usage            = false;

    if (argc > 3)
    {
        input_bin_file_name  = argv[1];
        output_bin_file_name = argv[2];
        if (!strcmp("NONE", argv[3]))
        {
            // Nothing to do here.
        }
        else if (!strcmp("LZ4", argv[3]))
        {
            compression_type       = brimstone::util::kLz4;
            dst_compression_string = argv[3];
        }
        else if (!strcmp("LZ77", argv[3]))
        {
            compression_type       = brimstone::util::kLz77;
            dst_compression_string = argv[3];
        }
        else
        {
            print_usage = true;
        }
    }
    else
    {
        print_usage = true;
    }

    if (print_usage)
    {
        std::string exe_name     = argv[0];
        size_t      dir_location = exe_name.find_last_of("/\\");
        if (dir_location >= 0)
        {
            exe_name.replace(0, dir_location + 1, "");
        }
        printf("\n%s\tis a compression/decompression tool for working with\n", exe_name.c_str());
        printf("\t\ttrace binary files.\n\n");
        printf("Usage:\n");
        printf("\t%s <input_bin> <output_bin> <output_compression>\n\n", exe_name.c_str());
        printf("\t<input_bin>\t\tThe filename (including path if necessary) of the \n");
        printf("\t\t\t\tincoming binary file to manipulate\n");
        printf("\t<output_bin>\t\tThe filename (including path if necessary) of the \n");
        printf("\t\t\t\tresulting binary file to generate\n");
        printf("\t<output_compression>\tThe compression to use when generating the\n");
        printf("\t\t\t\toutput file.  Possible values are: \n");
        printf("\t\t\t\t\tLZ4  - To output using LZ4 compression\n");
        printf("\t\t\t\t\tLZ77 - To output using LZ77 compression\n");
        printf("\t\t\t\t\tNONE - To output without using compression\n");
        exit(-1);
    }

    if (file_processor.Initialize(input_bin_file_name))
    {
        brimstone::format::DecompressDecoder decoder;

        if (decoder.Initialize(output_bin_file_name,
                               file_processor.GetFileHeader(),
                               file_processor.GetFileOptions(),
                               compression_type))
        {
            std::string src_compression = "NONE";
            file_processor.AddDecoder(&decoder);
            file_processor.ProcessAllFrames();

            for (auto& option : file_processor.GetFileOptions())
            {
                if (option.key == brimstone::format::kCompressionType)
                {
                    switch (option.value)
                    {
                        case brimstone::util::kNone:
                            // Nothing to do
                            break;
                        case brimstone::util::kLz4:
                            src_compression = "LZ4";
                            break;
                        case brimstone::util::kLz77:
                            src_compression = "LZ77";
                            break;
                        default:
                            printf("ERROR: Unknown source compression type %d", option.value);
                            assert(false);
                            break;
                    }
                }
            }

            if (brimstone::util::kNone != compression_type)
            {
                size_t bytes_read    = file_processor.NumBytesRead();
                size_t bytes_written = decoder.NumBytesWritten();
                float  percent_reduction =
                    100.f * (1.f - (static_cast<float>(bytes_written) / static_cast<float>(bytes_read)));
                printf("Compression Results:\n");
                printf("  Original Size   [Compression: %5s] = %lu bytes\n", src_compression.c_str(), bytes_read);
                printf("  Compressed Size [Compression: %5s] = %lu bytes\n",
                       dst_compression_string.c_str(),
                       bytes_written);
                printf("  Percent Reduction                    = %.2f%%\n", percent_reduction);
            }
            else
            {
                size_t bytes_read    = file_processor.NumBytesRead();
                size_t bytes_written = decoder.NumBytesWritten();
                float  percent_increase =
                    100.f * ((static_cast<float>(bytes_written) / static_cast<float>(bytes_read)) - 1.f);
                printf("Uncompression Results:\n");
                printf("  Original Size   [Compression: %5s] = %lu bytes\n", src_compression.c_str(), bytes_read);
                printf("  Uncompressed Size                    = %lu bytes\n", bytes_written);
                printf("  Percent Increase                     = %.2f%%\n", percent_increase);
            }
        }
    }

    return 0;
}