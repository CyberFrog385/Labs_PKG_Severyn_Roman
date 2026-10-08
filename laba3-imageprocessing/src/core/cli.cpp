#include "core/cli.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "core/clahe.h"
#include "core/image_io.h"
#include "core/processing.h"

namespace {

std::string get_param(int argc, char** argv, const char* key, const std::string& fallback) {
    std::string prefix = std::string(key) + "=";
    for (int i = 5; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.compare(0, prefix.size(), prefix) == 0) {
            return arg.substr(prefix.size());
        }
    }
    return fallback;
}

int get_param_int(int argc, char** argv, const char* key, int fallback) {
    std::string value = get_param(argc, argv, key, "");
    if (value.empty()) return fallback;
    return atoi(value.c_str());
}

double get_param_double(int argc, char** argv, const char* key, double fallback) {
    std::string value = get_param(argc, argv, key, "");
    if (value.empty()) return fallback;
    return atof(value.c_str());
}

void print_statistics(const char* title, const ImageBuffer& image) {
    double stats[4];
    compute_statistics(image, -1, stats);
    printf("%s: яркость min=%.0f max=%.0f среднее=%.2f sigma=%.2f",
           title, stats[0], stats[1], stats[2], stats[3]);
    if (image.channels() > 1) {
        for (int c = 0; c < image.channels(); ++c) {
            compute_statistics(image, c, stats);
            printf(" | K%d: %.0f..%.0f c=%.2f", c, stats[0], stats[1], stats[2]);
        }
    }
    printf("\n");
}

void print_usage() {
    printf("Использование:\n");
    printf("  ImageProcessing cli <вход> <выход.png> <операция> [ключ=значение ...]\n");
    printf("Операции:\n");
    printf("  contrast-auto\n");
    printf("  contrast-manual in_min=.. in_max=..\n");
    printf("  equalize-rgb\n");
    printf("  equalize-hsv\n");
    printf("  clahe tiles=.. clip=..\n");
    printf("  add value=.. | sub value=.. | mul factor=.. | div divisor=..\n");
    printf("  gamma gamma=.. | neg\n");
    printf("  and value=.. | or value=.. | xor value=..\n");
    printf("  hist\n");
}

ImageBuffer run_operation(const std::string& op,
                          const ImageBuffer& input,
                          int argc,
                          char** argv,
                          bool* ok) {
    *ok = true;
    if (op == "contrast-auto") return linear_contrast_auto(input, NULL);
    if (op == "contrast-manual") {
        int in_min = get_param_int(argc, argv, "in_min", 0);
        int in_max = get_param_int(argc, argv, "in_max", 255);
        return linear_contrast_manual(input, in_min, in_max, NULL);
    }
    if (op == "equalize-rgb") return equalize_rgb(input, NULL);
    if (op == "equalize-hsv") return equalize_hsv(input, NULL);
    if (op == "clahe") {
        int tiles = get_param_int(argc, argv, "tiles", 8);
        double clip = get_param_double(argc, argv, "clip", 2.0);
        return clahe(input, tiles, clip, NULL);
    }
    if (op == "add") return pixel_add_constant(input, get_param_int(argc, argv, "value", 30), NULL);
    if (op == "sub") return pixel_subtract_constant(input, get_param_int(argc, argv, "value", 30), NULL);
    if (op == "mul") return pixel_multiply_constant(input, get_param_double(argc, argv, "factor", 1.3), NULL);
    if (op == "div") return pixel_divide_constant(input, get_param_double(argc, argv, "divisor", 1.3), NULL);
    if (op == "gamma") return pixel_gamma(input, get_param_double(argc, argv, "gamma", 0.5), NULL);
    if (op == "neg") return pixel_negative(input, NULL);
    if (op == "and") return pixel_and_constant(input, get_param_int(argc, argv, "value", 240), NULL);
    if (op == "or") return pixel_or_constant(input, get_param_int(argc, argv, "value", 15), NULL);
    if (op == "xor") return pixel_xor_constant(input, get_param_int(argc, argv, "value", 255), NULL);
    if (op == "hist") return input;
    *ok = false;
    return ImageBuffer();
}

}

int run_cli(int argc, char** argv) {
    if (argc < 5) {
        print_usage();
        return 2;
    }
    std::string input_path = argv[2];
    std::string output_path = argv[3];
    std::string op = argv[4];

    std::string error;
    ImageBuffer input = load_image(input_path, &error);
    if (input.empty()) {
        fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    print_statistics("Вход ", input);

    if (op == "hist") {
        int histogram[256];
        compute_histogram(input, -1, histogram);
        FILE* file = fopen(output_path.c_str(), "w");
        if (!file) {
            fprintf(stderr, "Не удалось создать файл: %s\n", output_path.c_str());
            return 1;
        }
        for (int i = 0; i < 256; ++i) {
            fprintf(file, "%d;%d\n", i, histogram[i]);
        }
        fclose(file);
        printf("Гистограмма сохранена: %s\n", output_path.c_str());
        return 0;
    }

    bool ok = false;
    ImageBuffer result = run_operation(op, input, argc, argv, &ok);
    if (!ok) {
        fprintf(stderr, "Неизвестная операция: %s\n", op.c_str());
        print_usage();
        return 2;
    }

    print_statistics("Результат", result);

    if (!save_png(result, output_path, &error)) {
        fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    printf("Сохранено: %s\n", output_path.c_str());
    return 0;
}
