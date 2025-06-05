#include <stdio.h> 
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h> 
#include <stdint.h> 
#include <stdlib.h> 
#include <math.h> 
#include <time.h> 

#define WIDTH  1920
#define HEIGHT  1080
#define MAX_ITERATIONS 1000

typedef uint8_t channel; 

typedef struct Pixel_s {
    channel r; 
    channel g; 
    channel b; 
} Pixel_t ;

long double iteration_value(long double x0, long double y0)
{
    long double x = 0.0f; 
    long double y = 0.0f; 
    long double x2 = 0.0f; 
    long double y2 = 0.0f; 
    int iter = 0; 
    while (x2 + y2 <= 4 && iter < MAX_ITERATIONS)
    {
        y = 2 * x * y + y0;  
        x = x2 - y2 + x0; 
        x2 = x * x; 
        y2 = y * y; 
        iter++; 
    }

    if (iter == MAX_ITERATIONS) return (long double)MAX_ITERATIONS;

    long double mag2    = x2 + y2;                
    long double log_zn  = logf(mag2) * 0.5f;      
    long double nu      = logf(log_zn / logf(2.0f)) / logf(2.0f);
    return iter + 1.0f - nu;                
}

static Pixel_t hsv_to_rgb(long double h, long double s, long double v)
{
    long double c = v * s;
    long double hp = h / 60.0f;
    long double x = c * (1.0f - fabsf(fmodf(hp, 2.0f) - 1.0f));
    long double r=0, g=0, b=0;

    if      (hp < 1) { r = c; g = x; }
    else if (hp < 2) { r = x; g = c; }
    else if (hp < 3) { g = c; b = x; }
    else if (hp < 4) { g = x; b = c; }
    else if (hp < 5) { r = x; b = c; }
    else             { r = c; b = x; }

    long double m = v - c;
    return (Pixel_t){
        (channel)((r + m) * 255.0f + 0.5f),
        (channel)((g + m) * 255.0f + 0.5f),
        (channel)((b + m) * 255.0f + 0.5f)
    };
}

static Pixel_t colour_from_iteration(long double it)
{
    if (it >= MAX_ITERATIONS)           
        return (Pixel_t){0,0,0};

    long double t   = it / MAX_ITERATIONS;     
    long double hue = fmodf(t * 360.0f, 360.0f);
    return hsv_to_rgb(hue, 1.0f, 1.0f);
}


void save(Pixel_t* pixels, size_t width, size_t height, int frame_id) 
{
    char file_name[100]; 
    snprintf(file_name, 99, "frames/frame%04d.png", frame_id); 
    stbi_write_png(file_name, width, height, 3, pixels, width * 3);
}

void render(long double scale, int frame_id)
{
    Pixel_t* pixels = malloc(WIDTH * HEIGHT * sizeof(Pixel_t)); 
    //seahorse valley
    long double x_center = -0.743643887037151f;
    long double y_center = 0.131825904205330f;

    long double x_width = 3.5f * scale;
    long double y_height = x_width * HEIGHT / WIDTH;

    long double x_start = x_center - x_width / 2.0f;
    long double y_start = y_center - y_height / 2.0f;

    long double x_unit = x_width / WIDTH;
    long double y_unit = y_height / HEIGHT;

    clock_t start, end; 
    double cpu_time_used;

     start = clock();
#pragma omp parallel for schedule(static)
    for (int j = 0; j < HEIGHT; j++)
    {
        for (int i = 0; i < WIDTH; i++)
        {
            long double r_sum = 0, g_sum = 0, b_sum = 0;

            for (int dx = 0; dx < 2; ++dx) {
                for (int dy = 0; dy < 2; ++dy) {
                    long double x0 = x_start + (i + dx / 2.0) * x_unit;
                    long double y0 = y_start + (j + dy / 2.0) * y_unit;
                    long double it = iteration_value(x0, y0);
                    Pixel_t p = colour_from_iteration(it);
                    r_sum += p.r;
                    g_sum += p.g;
                    b_sum += p.b;
                }
            }

            pixels[j * WIDTH + i] = (Pixel_t){
                (channel)(r_sum / 4.0f + 0.5f),
                (channel)(g_sum / 4.0f + 0.5f),
                (channel)(b_sum / 4.0f + 0.5f)
            };
        }
    }
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("time taken : %fs\n", cpu_time_used); 
    save(pixels, WIDTH, HEIGHT, frame_id); 
    free(pixels); 
}

int main(void)
{
    long double initial_scale = 1.0; 
    long double zoom_factor = 0.96; 
    long double scale; 
    int total_frames = 9999; 
    for (int i = 0; i < total_frames; i++)
    {
        scale = initial_scale * powl(zoom_factor, i); 
        render(scale, i);
    }
    return 0; 
}
