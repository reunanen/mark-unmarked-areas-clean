#include <dlib/dir_nav/dir_nav_extensions.h>
#include <opencv2/imgcodecs/imgcodecs.hpp>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << std::endl;
        std::cout << "> mark-unmarked-areas-clean /path/to/anno/data" << std::endl;
        return 1;
    }

    const std::string mask_image_suffix = "_mask.png";

    std::cout << "Searching for images..." << std::endl;

    const std::vector<dlib::file> files = dlib::get_files_in_directory_tree(argv[1],
        [](const dlib::file& name) {
            if (dlib::match_ending("_mask.png")(name)) {
                return false;
            }
            if (dlib::match_ending("_result.png")(name)) {
                return false;
            }
            return dlib::match_ending(".jpeg")(name)
                || dlib::match_ending(".jpg")(name)
                || dlib::match_ending(".png")(name);
        });

    std::cout << "Processing " << files.size() << " images..." << std::endl;

    cv::Vec4b ignore_color(0, 0, 0, 0);
    cv::Vec4b clean_color(0, 255, 0, 64);

    for (const auto& file : files) {

        const std::string& full_name = file.full_name();

        std::cout << "Processing " << full_name;

        const std::string label_filename = full_name + "_mask.png";

        cv::Mat mask_image = cv::imread(label_filename, cv::IMREAD_UNCHANGED);

        if (!mask_image.data) {
            cv::Mat input_image = cv::imread(full_name);
            mask_image.create(input_image.size(), CV_8UC4);
            mask_image.setTo(clean_color);
            std::cout << ": generated a clean mask";
        }
        else {
            std::cout
                << ", width = " << mask_image.cols
                << ", height = " << mask_image.rows
                << ", channels = " << mask_image.channels()
                << ", type = 0x" << std::hex << mask_image.type();

            size_t converted_pixel_count = 0;
            for (int y = 0; y < mask_image.rows; ++y) {
                cv::Vec4b* row = mask_image.ptr<cv::Vec4b>(y);
                for (int x = 0; x < mask_image.cols; ++x) {
                    cv::Vec4b& color = row[x];
                    if (color == ignore_color) {
                        color = clean_color;
                        ++converted_pixel_count;
                    }
                }
            }

            std::cout << ": converted " << std::dec << converted_pixel_count << " pixels";
        }
        
        cv::imwrite(label_filename, mask_image);
        
        std::cout << std::endl;
    }
}
