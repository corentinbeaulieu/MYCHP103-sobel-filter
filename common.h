//Resolution
#define W 1280  //Width
#define H 720   //Height

//
#define MAX_SAMPLES 1000

//
#define INDEX(row, col, n) (((row) * (n)) + (col))

//
typedef unsigned char       u8;
typedef unsigned int       u32;
typedef unsigned long long u64;

//
typedef char      i8;
typedef short     i16;
typedef int       i32;
typedef long long i64;

//
typedef float  f32;
typedef double f64;

//
void sort(f64 *a, u64 n);

//Arithmetic mean
f64 mean(f64 *a, u64 n);

//Standard deviation
f64 stddev(f64 *a, u64 n);
