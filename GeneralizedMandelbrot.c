//The mandelbrot set is defined as all the points on the complex plane that do not
//tend to infinity when recursivly put through the formula z_(n+1) = (z_n)^2 + c.
//It is possible to generalize this function to z_(n+1) = (z_n)^m + c. This creates
//very interesting results. This program is written to calculate which points are included in
//and out of the generalized mandelbrot set, when given a window size and exponent.
//If given a range, it will calculate which points are in and out of the set, with
//the power increasing by a given increment. This data will then be written to a JSON file.


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

//Constants for the user to change
//Scale factor of the window
const int SCALE = 300;
//Starting and ending values for the function
const float START = -10, END = 10;
//The number of frames
const int DIVISIONS = 100000;

//Constants the user should NOT change
const int WIDTH = 3*SCALE, HEIGHT = 3*SCALE;
const float RANGE_X = 3.5, RANGE_Y = 3.5;
const float CENTER_X = 0, CENTER_Y = 0;
const float MIN_X = CENTER_X - RANGE_X/2.0;
const float MAX_X = CENTER_X + RANGE_X/2.0;
const float MIN_Y = CENTER_Y - RANGE_Y/2.0;
const float MAX_Y = CENTER_Y + RANGE_Y/2.0;
const float INCREMENT = (float)(END - START) / DIVISIONS;
const float PERFILE = 100.0;
const int MAX_I = 80;

struct Complex{
  float re;
  float im;
};

void Mandelbrot(float *values, float *histogram, float power, float cR, float cI);
void CalculateColors(float *values, float *histogram, float *arr);

float map(float var1, float start1, float end1, float start2, float end2);
float LinearInterpolation(float num1, float num2, float point);

struct Complex Alg(struct Complex com1, struct Complex com2, float power, float cR, float cI);

void StartWriteToJSON(int index);
void MiddleWriteToJSON(float *arr, int index);
void LastWriteToJSON(float *arr, int index);
void FinishWriteToJSON(int index);

char* GetPath(int index);

int main(){
    //initialization of variables
    float nums[WIDTH * HEIGHT];
    float numFiles = DIVISIONS / PERFILE;
    float values[WIDTH * HEIGHT];
    float histogram[MAX_I];
    int temp = 0;
    clock_t start, end;
    double cpuTimeUsed;
    int h, m, s;

    start = clock();

    //Runs the algorithm for each power in the range
    for(int i = 0; i < (int)numFiles + ceil((numFiles) - (int)numFiles); i++){
      StartWriteToJSON(i);
      if(i == 0){
        Mandelbrot(values, histogram, START + i * ((END - START) / numFiles));
        CalculateColors(values, histogram, nums);
        MiddleWriteToJSON(nums, i);
        printf("power: %f, %d/%d iterations, %f%%\n", START + i * ((END - START) / numFiles), 0, DIVISIONS, 0.0);
      }
      for(float j = START + i * ((END - START) / numFiles) + INCREMENT; ((int)(j * 100000 + 0.5))/100000.0 < (START + (i + 1) * ((END - START) / numFiles)); j += INCREMENT){
        j = (((int)(fabs(j) * 100000 + 0.5))/100000.0) * ((j > 0) ? 1 : -1);
        // Note: Power is divided by DIVISIONS
        Mandelbrot(values, histogram, j);
        CalculateColors(values, histogram, nums);
        MiddleWriteToJSON(nums, i);
        temp = (int)round(((j-START)/(float)(END - START)) * DIVISIONS);
        printf("power: %f, %d/%d iterations, %f%%\n", j, temp, DIVISIONS, (100.0 * temp) / DIVISIONS);
      }
      Mandelbrot(values, histogram, (START + (i + 1) * ((END - START) / numFiles)));
      CalculateColors(values, histogram, nums);
      LastWriteToJSON(nums, i);
      printf("power: %f, %d/%d iterations, %f%%\n", (START + (i + 1) * ((END - START) / numFiles)), (int)(((i + 1) * numFiles) / 10), DIVISIONS, (10 * (1 + i) * numFiles) / DIVISIONS);
      FinishWriteToJSON(i);
    }

    // StartWriteToJSON(1005);
    // Mandelbrot(values, histogram, -9.5);
    // CalculateColors(values, histogram, nums);
    // LastWriteToJSON(nums, 1005);
    // FinishWriteToJSON(1005);

    // StartWriteToJSON(0);
    // for(int i = -5; i < 5; i++){
    //   Mandelbrot(values, histogram, i);
    //   CalculateColors(values, histogram, nums);
    //   MiddleWriteToJSON(nums, 0);
    //   printf("done\n");
    // }
    // Mandelbrot(values, histogram, 5);
    // CalculateColors(values, histogram, nums);
    // LastWriteToJSON(nums, 0);
    // FinishWriteToJSON(0);

    printf("Done!\n");
    end = clock();
    cpuTimeUsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    h = (cpuTimeUsed/3600);
    m = (cpuTimeUsed -(3600*h))/60;
	  s = (cpuTimeUsed -(3600*h)-(m*60));
    printf("The program took %d:%d:%d to run.\n", h, m, s);
    return 0;
}

void Mandelbrot(float *values, float *histogram, float power, float cR, float cI){
  struct Complex com1;
  struct Complex com2;
  int n = 0;
  float re, im;

  //Runs the algorithm for each pixel on the screen, mapped between the constraints
  for(int i = 0; i < WIDTH; i++){
    for(int j = 0; j < HEIGHT; j++){
      re = map(i, 0, WIDTH, MIN_X, MAX_X);
      im = map(j, 0, HEIGHT, MIN_Y, MAX_Y);

      com1.re = re;
      com1.im = im;
      com2.re = re;
      com2.im = im;

      n = 0;

      //If the modulus of the complex number (the distance between it and the origin) is
      //greater than 4, break b/c it will go to infinity. If the point has reached n, it is considered 'in'.
      while(n < MAX_I && sqrt(com1.re * com1.re + com1.im * com1.im) < 4) {
        // printf("%f + %fi, %f + %fi\n", com1.re, com1.im, Power(com1, power).re, Power(com1, power).im);
        com1 = Alg(com1, com2, power, cR, cI);

        n++;
      }

      //Calculates data for the color algorithm
      values[i * HEIGHT + j] = n;
      if(n < MAX_I){
        histogram[(int)n]++;
      }
    }
  }
}

//Color algorithm to eleminate stark borders in the visualization.
//I got this from Wikipedia I think, I honestly can't remember how it works now :S
void CalculateColors(float *values, float *histogram, float *arr){
  int total = 0;
  float hues[MAX_I];
  float h = 0;
  for(int i = 0; i < MAX_I; i++){
    total += histogram[i];
  }

  for(int i = 0; i < MAX_I; i++){
    h += histogram[i] / total;
    hues[i] = h;
  }
  hues[MAX_I - 1] = h;

  // printf("%f + %fi, %d\n", com1.re, com1.im, n);
  for(int i = 0; i < WIDTH; i++){
    for(int j = 0; j < HEIGHT; j++){
      float currentVal = values[i * HEIGHT + j];
      if((int)currentVal == MAX_I) {
        arr[i * HEIGHT + j] = NAN;
      }else {
        // arr[i * HEIGHT + j] = (255 * (n + 1 - log(log2(sqrt(com1.re * com1.re + com1.im * com1.im))))) / MAX_I;
        arr[i * HEIGHT + j] = 255 - (255 * LinearInterpolation(hues[(int)currentVal], hues[(int)ceil(currentVal)], (int)currentVal % 1));
      }
    }
  }
}

//var1 is to (end1 - start1) as RETURN is to (end2 - start2)
float map(float var1, float start1, float end1, float start2, float end2){
  return start2 + (var1 * (fabs(end2) + fabs(start2))) / (fabs(end1) + fabs(start1));
}

float LinearInterpolation(float num1, float num2, float point){
  return num1 * (1 - point) + num2 * point;
}

//Calculates a complex number to a non-integer power, adds a specified point
struct Complex Alg(struct Complex com1, struct Complex com2, float power, float cR, float cI){
  if(com1.re == 0.0 && com1.im == 0) return com1;
  float r = pow(com1.re * com1.re + com1.im * com1.im, power / 2.0);
  float theta = power * atan2(com1.im, com1.re);
  struct Complex c = {r * cos(theta) + com2.re + cR, r * sin(theta) + com2.im + cI};
  return c;
}

void StartWriteToJSON(int index){
  FILE *fp;
  fp = fopen(GetPath(index), "w");

  fprintf(fp, "{\n\t\"width\": %d,\n\t\"height\": %d,\n\t\"iterations\": %f,\n\t\"nums\": [\n", WIDTH, HEIGHT, PERFILE);

  fclose(fp);
}

void MiddleWriteToJSON(float *arr, int index){
  FILE *fp;
  fp = fopen(GetPath(index), "a");

  fputs("\t\t[", fp);

  for(int i = 0; i < WIDTH * HEIGHT - 1; i++){
    if(arr[i] == arr[i]){
      fprintf(fp, "%f", arr[i]);
    }else{
      fputs("NaN", fp);
    }

    fputs(", ", fp);
  }

  fprintf(fp, "%f", arr[WIDTH * HEIGHT]);

  fputs("\t],\n", fp);

  fclose(fp);
}

void LastWriteToJSON(float *arr, int index){
  FILE *fp;
  fp = fopen(GetPath(index), "a");

  fputs("\t\t[", fp);

  for(int i = 0; i < WIDTH * HEIGHT - 1; i++){
    if(arr[i] == arr[i]){
      fprintf(fp, "%f", arr[i]);
    }else{
      fputs("NaN", fp);
    }

    fputs(", ", fp);
  }

  fprintf(fp, "%f", arr[WIDTH * HEIGHT]);

  fputs("\t]\n", fp);

  fclose(fp);
}

void FinishWriteToJSON(int index){
  FILE *fp;
  fp = fopen(GetPath(index), "a");

  fputs("\t]\n}", fp);

  fclose(fp);
}

//No, I couldn't think of a better way of doing it.
//No, I don't want to talk about it.
//Yes, I did use Python to write this.
//You need to have Einstein like IQ to figure out how to add two strings in C.
char* GetPath(int index){
  if(index == 0) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_0.json";
	if(index == 1) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_1.json";
	if(index == 2) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_2.json";
	if(index == 3) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_3.json";
	if(index == 4) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_4.json";
	if(index == 5) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_5.json";
	if(index == 6) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_6.json";
	if(index == 7) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_7.json";
	if(index == 8) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_8.json";
	if(index == 9) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_9.json";
	if(index == 10) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_10.json";
	if(index == 11) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_11.json";
	if(index == 12) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_12.json";
	if(index == 13) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_13.json";
	if(index == 14) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_14.json";
	if(index == 15) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_15.json";
	if(index == 16) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_16.json";
	if(index == 17) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_17.json";
	if(index == 18) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_18.json";
	if(index == 19) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_19.json";
	if(index == 20) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_20.json";
	if(index == 21) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_21.json";
	if(index == 22) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_22.json";
	if(index == 23) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_23.json";
	if(index == 24) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_24.json";
	if(index == 25) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_25.json";
	if(index == 26) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_26.json";
	if(index == 27) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_27.json";
	if(index == 28) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_28.json";
	if(index == 29) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_29.json";
	if(index == 30) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_30.json";
	if(index == 31) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_31.json";
	if(index == 32) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_32.json";
	if(index == 33) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_33.json";
	if(index == 34) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_34.json";
	if(index == 35) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_35.json";
	if(index == 36) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_36.json";
	if(index == 37) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_37.json";
	if(index == 38) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_38.json";
	if(index == 39) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_39.json";
	if(index == 40) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_40.json";
	if(index == 41) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_41.json";
	if(index == 42) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_42.json";
	if(index == 43) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_43.json";
	if(index == 44) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_44.json";
	if(index == 45) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_45.json";
	if(index == 46) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_46.json";
	if(index == 47) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_47.json";
	if(index == 48) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_48.json";
	if(index == 49) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_49.json";
	if(index == 50) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_50.json";
	if(index == 51) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_51.json";
	if(index == 52) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_52.json";
	if(index == 53) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_53.json";
	if(index == 54) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_54.json";
	if(index == 55) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_55.json";
	if(index == 56) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_56.json";
	if(index == 57) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_57.json";
	if(index == 58) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_58.json";
	if(index == 59) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_59.json";
	if(index == 60) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_60.json";
	if(index == 61) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_61.json";
	if(index == 62) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_62.json";
	if(index == 63) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_63.json";
	if(index == 64) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_64.json";
	if(index == 65) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_65.json";
	if(index == 66) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_66.json";
	if(index == 67) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_67.json";
	if(index == 68) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_68.json";
	if(index == 69) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_69.json";
	if(index == 70) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_70.json";
	if(index == 71) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_71.json";
	if(index == 72) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_72.json";
	if(index == 73) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_73.json";
	if(index == 74) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_74.json";
	if(index == 75) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_75.json";
	if(index == 76) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_76.json";
	if(index == 77) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_77.json";
	if(index == 78) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_78.json";
	if(index == 79) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_79.json";
	if(index == 80) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_80.json";
	if(index == 81) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_81.json";
	if(index == 82) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_82.json";
	if(index == 83) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_83.json";
	if(index == 84) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_84.json";
	if(index == 85) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_85.json";
	if(index == 86) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_86.json";
	if(index == 87) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_87.json";
	if(index == 88) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_88.json";
	if(index == 89) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_89.json";
	if(index == 90) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_90.json";
	if(index == 91) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_91.json";
	if(index == 92) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_92.json";
	if(index == 93) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_93.json";
	if(index == 94) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_94.json";
	if(index == 95) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_95.json";
	if(index == 96) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_96.json";
	if(index == 97) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_97.json";
	if(index == 98) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_98.json";
	if(index == 99) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_99.json";
	if(index == 100) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_100.json";
	if(index == 101) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_101.json";
	if(index == 102) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_102.json";
	if(index == 103) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_103.json";
	if(index == 104) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_104.json";
	if(index == 105) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_105.json";
	if(index == 106) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_106.json";
	if(index == 107) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_107.json";
	if(index == 108) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_108.json";
	if(index == 109) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_109.json";
	if(index == 110) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_110.json";
	if(index == 111) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_111.json";
	if(index == 112) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_112.json";
	if(index == 113) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_113.json";
	if(index == 114) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_114.json";
	if(index == 115) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_115.json";
	if(index == 116) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_116.json";
	if(index == 117) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_117.json";
	if(index == 118) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_118.json";
	if(index == 119) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_119.json";
	if(index == 120) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_120.json";
	if(index == 121) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_121.json";
	if(index == 122) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_122.json";
	if(index == 123) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_123.json";
	if(index == 124) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_124.json";
	if(index == 125) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_125.json";
	if(index == 126) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_126.json";
	if(index == 127) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_127.json";
	if(index == 128) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_128.json";
	if(index == 129) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_129.json";
	if(index == 130) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_130.json";
	if(index == 131) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_131.json";
	if(index == 132) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_132.json";
	if(index == 133) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_133.json";
	if(index == 134) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_134.json";
	if(index == 135) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_135.json";
	if(index == 136) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_136.json";
	if(index == 137) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_137.json";
	if(index == 138) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_138.json";
	if(index == 139) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_139.json";
	if(index == 140) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_140.json";
	if(index == 141) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_141.json";
	if(index == 142) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_142.json";
	if(index == 143) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_143.json";
	if(index == 144) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_144.json";
	if(index == 145) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_145.json";
	if(index == 146) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_146.json";
	if(index == 147) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_147.json";
	if(index == 148) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_148.json";
	if(index == 149) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_149.json";
	if(index == 150) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_150.json";
	if(index == 151) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_151.json";
	if(index == 152) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_152.json";
	if(index == 153) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_153.json";
	if(index == 154) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_154.json";
	if(index == 155) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_155.json";
	if(index == 156) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_156.json";
	if(index == 157) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_157.json";
	if(index == 158) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_158.json";
	if(index == 159) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_159.json";
	if(index == 160) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_160.json";
	if(index == 161) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_161.json";
	if(index == 162) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_162.json";
	if(index == 163) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_163.json";
	if(index == 164) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_164.json";
	if(index == 165) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_165.json";
	if(index == 166) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_166.json";
	if(index == 167) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_167.json";
	if(index == 168) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_168.json";
	if(index == 169) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_169.json";
	if(index == 170) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_170.json";
	if(index == 171) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_171.json";
	if(index == 172) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_172.json";
	if(index == 173) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_173.json";
	if(index == 174) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_174.json";
	if(index == 175) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_175.json";
	if(index == 176) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_176.json";
	if(index == 177) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_177.json";
	if(index == 178) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_178.json";
	if(index == 179) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_179.json";
	if(index == 180) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_180.json";
	if(index == 181) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_181.json";
	if(index == 182) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_182.json";
	if(index == 183) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_183.json";
	if(index == 184) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_184.json";
	if(index == 185) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_185.json";
	if(index == 186) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_186.json";
	if(index == 187) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_187.json";
	if(index == 188) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_188.json";
	if(index == 189) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_189.json";
	if(index == 190) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_190.json";
	if(index == 191) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_191.json";
	if(index == 192) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_192.json";
	if(index == 193) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_193.json";
	if(index == 194) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_194.json";
	if(index == 195) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_195.json";
	if(index == 196) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_196.json";
	if(index == 197) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_197.json";
	if(index == 198) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_198.json";
	if(index == 199) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_199.json";
	if(index == 200) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_200.json";
	if(index == 201) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_201.json";
	if(index == 202) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_202.json";
	if(index == 203) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_203.json";
	if(index == 204) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_204.json";
	if(index == 205) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_205.json";
	if(index == 206) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_206.json";
	if(index == 207) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_207.json";
	if(index == 208) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_208.json";
	if(index == 209) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_209.json";
	if(index == 210) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_210.json";
	if(index == 211) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_211.json";
	if(index == 212) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_212.json";
	if(index == 213) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_213.json";
	if(index == 214) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_214.json";
	if(index == 215) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_215.json";
	if(index == 216) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_216.json";
	if(index == 217) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_217.json";
	if(index == 218) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_218.json";
	if(index == 219) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_219.json";
	if(index == 220) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_220.json";
	if(index == 221) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_221.json";
	if(index == 222) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_222.json";
	if(index == 223) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_223.json";
	if(index == 224) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_224.json";
	if(index == 225) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_225.json";
	if(index == 226) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_226.json";
	if(index == 227) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_227.json";
	if(index == 228) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_228.json";
	if(index == 229) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_229.json";
	if(index == 230) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_230.json";
	if(index == 231) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_231.json";
	if(index == 232) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_232.json";
	if(index == 233) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_233.json";
	if(index == 234) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_234.json";
	if(index == 235) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_235.json";
	if(index == 236) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_236.json";
	if(index == 237) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_237.json";
	if(index == 238) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_238.json";
	if(index == 239) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_239.json";
	if(index == 240) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_240.json";
	if(index == 241) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_241.json";
	if(index == 242) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_242.json";
	if(index == 243) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_243.json";
	if(index == 244) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_244.json";
	if(index == 245) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_245.json";
	if(index == 246) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_246.json";
	if(index == 247) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_247.json";
	if(index == 248) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_248.json";
	if(index == 249) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_249.json";
	if(index == 250) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_250.json";
	if(index == 251) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_251.json";
	if(index == 252) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_252.json";
	if(index == 253) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_253.json";
	if(index == 254) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_254.json";
	if(index == 255) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_255.json";
	if(index == 256) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_256.json";
	if(index == 257) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_257.json";
	if(index == 258) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_258.json";
	if(index == 259) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_259.json";
	if(index == 260) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_260.json";
	if(index == 261) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_261.json";
	if(index == 262) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_262.json";
	if(index == 263) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_263.json";
	if(index == 264) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_264.json";
	if(index == 265) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_265.json";
	if(index == 266) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_266.json";
	if(index == 267) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_267.json";
	if(index == 268) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_268.json";
	if(index == 269) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_269.json";
	if(index == 270) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_270.json";
	if(index == 271) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_271.json";
	if(index == 272) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_272.json";
	if(index == 273) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_273.json";
	if(index == 274) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_274.json";
	if(index == 275) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_275.json";
	if(index == 276) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_276.json";
	if(index == 277) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_277.json";
	if(index == 278) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_278.json";
	if(index == 279) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_279.json";
	if(index == 280) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_280.json";
	if(index == 281) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_281.json";
	if(index == 282) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_282.json";
	if(index == 283) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_283.json";
	if(index == 284) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_284.json";
	if(index == 285) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_285.json";
	if(index == 286) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_286.json";
	if(index == 287) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_287.json";
	if(index == 288) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_288.json";
	if(index == 289) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_289.json";
	if(index == 290) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_290.json";
	if(index == 291) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_291.json";
	if(index == 292) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_292.json";
	if(index == 293) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_293.json";
	if(index == 294) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_294.json";
	if(index == 295) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_295.json";
	if(index == 296) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_296.json";
	if(index == 297) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_297.json";
	if(index == 298) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_298.json";
	if(index == 299) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_299.json";
	if(index == 300) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_300.json";
	if(index == 301) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_301.json";
	if(index == 302) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_302.json";
	if(index == 303) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_303.json";
	if(index == 304) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_304.json";
	if(index == 305) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_305.json";
	if(index == 306) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_306.json";
	if(index == 307) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_307.json";
	if(index == 308) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_308.json";
	if(index == 309) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_309.json";
	if(index == 310) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_310.json";
	if(index == 311) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_311.json";
	if(index == 312) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_312.json";
	if(index == 313) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_313.json";
	if(index == 314) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_314.json";
	if(index == 315) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_315.json";
	if(index == 316) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_316.json";
	if(index == 317) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_317.json";
	if(index == 318) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_318.json";
	if(index == 319) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_319.json";
	if(index == 320) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_320.json";
	if(index == 321) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_321.json";
	if(index == 322) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_322.json";
	if(index == 323) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_323.json";
	if(index == 324) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_324.json";
	if(index == 325) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_325.json";
	if(index == 326) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_326.json";
	if(index == 327) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_327.json";
	if(index == 328) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_328.json";
	if(index == 329) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_329.json";
	if(index == 330) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_330.json";
	if(index == 331) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_331.json";
	if(index == 332) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_332.json";
	if(index == 333) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_333.json";
	if(index == 334) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_334.json";
	if(index == 335) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_335.json";
	if(index == 336) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_336.json";
	if(index == 337) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_337.json";
	if(index == 338) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_338.json";
	if(index == 339) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_339.json";
	if(index == 340) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_340.json";
	if(index == 341) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_341.json";
	if(index == 342) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_342.json";
	if(index == 343) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_343.json";
	if(index == 344) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_344.json";
	if(index == 345) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_345.json";
	if(index == 346) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_346.json";
	if(index == 347) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_347.json";
	if(index == 348) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_348.json";
	if(index == 349) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_349.json";
	if(index == 350) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_350.json";
	if(index == 351) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_351.json";
	if(index == 352) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_352.json";
	if(index == 353) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_353.json";
	if(index == 354) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_354.json";
	if(index == 355) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_355.json";
	if(index == 356) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_356.json";
	if(index == 357) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_357.json";
	if(index == 358) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_358.json";
	if(index == 359) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_359.json";
	if(index == 360) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_360.json";
	if(index == 361) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_361.json";
	if(index == 362) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_362.json";
	if(index == 363) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_363.json";
	if(index == 364) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_364.json";
	if(index == 365) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_365.json";
	if(index == 366) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_366.json";
	if(index == 367) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_367.json";
	if(index == 368) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_368.json";
	if(index == 369) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_369.json";
	if(index == 370) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_370.json";
	if(index == 371) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_371.json";
	if(index == 372) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_372.json";
	if(index == 373) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_373.json";
	if(index == 374) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_374.json";
	if(index == 375) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_375.json";
	if(index == 376) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_376.json";
	if(index == 377) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_377.json";
	if(index == 378) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_378.json";
	if(index == 379) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_379.json";
	if(index == 380) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_380.json";
	if(index == 381) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_381.json";
	if(index == 382) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_382.json";
	if(index == 383) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_383.json";
	if(index == 384) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_384.json";
	if(index == 385) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_385.json";
	if(index == 386) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_386.json";
	if(index == 387) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_387.json";
	if(index == 388) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_388.json";
	if(index == 389) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_389.json";
	if(index == 390) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_390.json";
	if(index == 391) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_391.json";
	if(index == 392) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_392.json";
	if(index == 393) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_393.json";
	if(index == 394) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_394.json";
	if(index == 395) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_395.json";
	if(index == 396) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_396.json";
	if(index == 397) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_397.json";
	if(index == 398) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_398.json";
	if(index == 399) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_399.json";
	if(index == 400) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_400.json";
	if(index == 401) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_401.json";
	if(index == 402) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_402.json";
	if(index == 403) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_403.json";
	if(index == 404) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_404.json";
	if(index == 405) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_405.json";
	if(index == 406) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_406.json";
	if(index == 407) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_407.json";
	if(index == 408) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_408.json";
	if(index == 409) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_409.json";
	if(index == 410) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_410.json";
	if(index == 411) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_411.json";
	if(index == 412) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_412.json";
	if(index == 413) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_413.json";
	if(index == 414) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_414.json";
	if(index == 415) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_415.json";
	if(index == 416) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_416.json";
	if(index == 417) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_417.json";
	if(index == 418) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_418.json";
	if(index == 419) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_419.json";
	if(index == 420) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_420.json";
	if(index == 421) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_421.json";
	if(index == 422) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_422.json";
	if(index == 423) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_423.json";
	if(index == 424) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_424.json";
	if(index == 425) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_425.json";
	if(index == 426) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_426.json";
	if(index == 427) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_427.json";
	if(index == 428) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_428.json";
	if(index == 429) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_429.json";
	if(index == 430) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_430.json";
	if(index == 431) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_431.json";
	if(index == 432) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_432.json";
	if(index == 433) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_433.json";
	if(index == 434) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_434.json";
	if(index == 435) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_435.json";
	if(index == 436) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_436.json";
	if(index == 437) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_437.json";
	if(index == 438) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_438.json";
	if(index == 439) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_439.json";
	if(index == 440) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_440.json";
	if(index == 441) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_441.json";
	if(index == 442) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_442.json";
	if(index == 443) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_443.json";
	if(index == 444) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_444.json";
	if(index == 445) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_445.json";
	if(index == 446) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_446.json";
	if(index == 447) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_447.json";
	if(index == 448) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_448.json";
	if(index == 449) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_449.json";
	if(index == 450) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_450.json";
	if(index == 451) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_451.json";
	if(index == 452) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_452.json";
	if(index == 453) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_453.json";
	if(index == 454) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_454.json";
	if(index == 455) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_455.json";
	if(index == 456) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_456.json";
	if(index == 457) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_457.json";
	if(index == 458) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_458.json";
	if(index == 459) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_459.json";
	if(index == 460) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_460.json";
	if(index == 461) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_461.json";
	if(index == 462) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_462.json";
	if(index == 463) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_463.json";
	if(index == 464) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_464.json";
	if(index == 465) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_465.json";
	if(index == 466) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_466.json";
	if(index == 467) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_467.json";
	if(index == 468) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_468.json";
	if(index == 469) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_469.json";
	if(index == 470) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_470.json";
	if(index == 471) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_471.json";
	if(index == 472) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_472.json";
	if(index == 473) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_473.json";
	if(index == 474) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_474.json";
	if(index == 475) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_475.json";
	if(index == 476) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_476.json";
	if(index == 477) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_477.json";
	if(index == 478) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_478.json";
	if(index == 479) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_479.json";
	if(index == 480) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_480.json";
	if(index == 481) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_481.json";
	if(index == 482) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_482.json";
	if(index == 483) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_483.json";
	if(index == 484) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_484.json";
	if(index == 485) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_485.json";
	if(index == 486) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_486.json";
	if(index == 487) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_487.json";
	if(index == 488) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_488.json";
	if(index == 489) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_489.json";
	if(index == 490) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_490.json";
	if(index == 491) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_491.json";
	if(index == 492) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_492.json";
	if(index == 493) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_493.json";
	if(index == 494) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_494.json";
	if(index == 495) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_495.json";
	if(index == 496) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_496.json";
	if(index == 497) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_497.json";
	if(index == 498) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_498.json";
	if(index == 499) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_499.json";
	if(index == 500) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_500.json";
	if(index == 501) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_501.json";
	if(index == 502) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_502.json";
	if(index == 503) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_503.json";
	if(index == 504) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_504.json";
	if(index == 505) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_505.json";
	if(index == 506) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_506.json";
	if(index == 507) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_507.json";
	if(index == 508) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_508.json";
	if(index == 509) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_509.json";
	if(index == 510) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_510.json";
	if(index == 511) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_511.json";
	if(index == 512) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_512.json";
	if(index == 513) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_513.json";
	if(index == 514) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_514.json";
	if(index == 515) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_515.json";
	if(index == 516) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_516.json";
	if(index == 517) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_517.json";
	if(index == 518) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_518.json";
	if(index == 519) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_519.json";
	if(index == 520) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_520.json";
	if(index == 521) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_521.json";
	if(index == 522) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_522.json";
	if(index == 523) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_523.json";
	if(index == 524) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_524.json";
	if(index == 525) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_525.json";
	if(index == 526) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_526.json";
	if(index == 527) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_527.json";
	if(index == 528) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_528.json";
	if(index == 529) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_529.json";
	if(index == 530) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_530.json";
	if(index == 531) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_531.json";
	if(index == 532) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_532.json";
	if(index == 533) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_533.json";
	if(index == 534) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_534.json";
	if(index == 535) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_535.json";
	if(index == 536) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_536.json";
	if(index == 537) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_537.json";
	if(index == 538) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_538.json";
	if(index == 539) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_539.json";
	if(index == 540) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_540.json";
	if(index == 541) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_541.json";
	if(index == 542) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_542.json";
	if(index == 543) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_543.json";
	if(index == 544) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_544.json";
	if(index == 545) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_545.json";
	if(index == 546) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_546.json";
	if(index == 547) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_547.json";
	if(index == 548) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_548.json";
	if(index == 549) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_549.json";
	if(index == 550) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_550.json";
	if(index == 551) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_551.json";
	if(index == 552) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_552.json";
	if(index == 553) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_553.json";
	if(index == 554) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_554.json";
	if(index == 555) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_555.json";
	if(index == 556) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_556.json";
	if(index == 557) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_557.json";
	if(index == 558) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_558.json";
	if(index == 559) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_559.json";
	if(index == 560) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_560.json";
	if(index == 561) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_561.json";
	if(index == 562) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_562.json";
	if(index == 563) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_563.json";
	if(index == 564) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_564.json";
	if(index == 565) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_565.json";
	if(index == 566) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_566.json";
	if(index == 567) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_567.json";
	if(index == 568) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_568.json";
	if(index == 569) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_569.json";
	if(index == 570) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_570.json";
	if(index == 571) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_571.json";
	if(index == 572) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_572.json";
	if(index == 573) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_573.json";
	if(index == 574) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_574.json";
	if(index == 575) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_575.json";
	if(index == 576) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_576.json";
	if(index == 577) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_577.json";
	if(index == 578) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_578.json";
	if(index == 579) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_579.json";
	if(index == 580) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_580.json";
	if(index == 581) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_581.json";
	if(index == 582) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_582.json";
	if(index == 583) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_583.json";
	if(index == 584) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_584.json";
	if(index == 585) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_585.json";
	if(index == 586) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_586.json";
	if(index == 587) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_587.json";
	if(index == 588) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_588.json";
	if(index == 589) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_589.json";
	if(index == 590) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_590.json";
	if(index == 591) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_591.json";
	if(index == 592) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_592.json";
	if(index == 593) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_593.json";
	if(index == 594) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_594.json";
	if(index == 595) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_595.json";
	if(index == 596) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_596.json";
	if(index == 597) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_597.json";
	if(index == 598) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_598.json";
	if(index == 599) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_599.json";
	if(index == 600) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_600.json";
	if(index == 601) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_601.json";
	if(index == 602) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_602.json";
	if(index == 603) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_603.json";
	if(index == 604) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_604.json";
	if(index == 605) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_605.json";
	if(index == 606) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_606.json";
	if(index == 607) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_607.json";
	if(index == 608) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_608.json";
	if(index == 609) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_609.json";
	if(index == 610) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_610.json";
	if(index == 611) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_611.json";
	if(index == 612) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_612.json";
	if(index == 613) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_613.json";
	if(index == 614) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_614.json";
	if(index == 615) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_615.json";
	if(index == 616) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_616.json";
	if(index == 617) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_617.json";
	if(index == 618) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_618.json";
	if(index == 619) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_619.json";
	if(index == 620) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_620.json";
	if(index == 621) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_621.json";
	if(index == 622) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_622.json";
	if(index == 623) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_623.json";
	if(index == 624) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_624.json";
	if(index == 625) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_625.json";
	if(index == 626) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_626.json";
	if(index == 627) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_627.json";
	if(index == 628) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_628.json";
	if(index == 629) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_629.json";
	if(index == 630) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_630.json";
	if(index == 631) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_631.json";
	if(index == 632) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_632.json";
	if(index == 633) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_633.json";
	if(index == 634) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_634.json";
	if(index == 635) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_635.json";
	if(index == 636) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_636.json";
	if(index == 637) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_637.json";
	if(index == 638) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_638.json";
	if(index == 639) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_639.json";
	if(index == 640) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_640.json";
	if(index == 641) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_641.json";
	if(index == 642) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_642.json";
	if(index == 643) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_643.json";
	if(index == 644) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_644.json";
	if(index == 645) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_645.json";
	if(index == 646) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_646.json";
	if(index == 647) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_647.json";
	if(index == 648) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_648.json";
	if(index == 649) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_649.json";
	if(index == 650) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_650.json";
	if(index == 651) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_651.json";
	if(index == 652) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_652.json";
	if(index == 653) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_653.json";
	if(index == 654) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_654.json";
	if(index == 655) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_655.json";
	if(index == 656) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_656.json";
	if(index == 657) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_657.json";
	if(index == 658) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_658.json";
	if(index == 659) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_659.json";
	if(index == 660) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_660.json";
	if(index == 661) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_661.json";
	if(index == 662) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_662.json";
	if(index == 663) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_663.json";
	if(index == 664) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_664.json";
	if(index == 665) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_665.json";
	if(index == 666) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_666.json";
	if(index == 667) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_667.json";
	if(index == 668) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_668.json";
	if(index == 669) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_669.json";
	if(index == 670) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_670.json";
	if(index == 671) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_671.json";
	if(index == 672) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_672.json";
	if(index == 673) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_673.json";
	if(index == 674) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_674.json";
	if(index == 675) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_675.json";
	if(index == 676) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_676.json";
	if(index == 677) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_677.json";
	if(index == 678) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_678.json";
	if(index == 679) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_679.json";
	if(index == 680) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_680.json";
	if(index == 681) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_681.json";
	if(index == 682) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_682.json";
	if(index == 683) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_683.json";
	if(index == 684) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_684.json";
	if(index == 685) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_685.json";
	if(index == 686) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_686.json";
	if(index == 687) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_687.json";
	if(index == 688) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_688.json";
	if(index == 689) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_689.json";
	if(index == 690) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_690.json";
	if(index == 691) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_691.json";
	if(index == 692) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_692.json";
	if(index == 693) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_693.json";
	if(index == 694) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_694.json";
	if(index == 695) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_695.json";
	if(index == 696) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_696.json";
	if(index == 697) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_697.json";
	if(index == 698) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_698.json";
	if(index == 699) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_699.json";
	if(index == 700) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_700.json";
	if(index == 701) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_701.json";
	if(index == 702) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_702.json";
	if(index == 703) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_703.json";
	if(index == 704) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_704.json";
	if(index == 705) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_705.json";
	if(index == 706) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_706.json";
	if(index == 707) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_707.json";
	if(index == 708) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_708.json";
	if(index == 709) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_709.json";
	if(index == 710) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_710.json";
	if(index == 711) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_711.json";
	if(index == 712) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_712.json";
	if(index == 713) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_713.json";
	if(index == 714) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_714.json";
	if(index == 715) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_715.json";
	if(index == 716) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_716.json";
	if(index == 717) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_717.json";
	if(index == 718) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_718.json";
	if(index == 719) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_719.json";
	if(index == 720) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_720.json";
	if(index == 721) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_721.json";
	if(index == 722) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_722.json";
	if(index == 723) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_723.json";
	if(index == 724) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_724.json";
	if(index == 725) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_725.json";
	if(index == 726) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_726.json";
	if(index == 727) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_727.json";
	if(index == 728) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_728.json";
	if(index == 729) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_729.json";
	if(index == 730) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_730.json";
	if(index == 731) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_731.json";
	if(index == 732) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_732.json";
	if(index == 733) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_733.json";
	if(index == 734) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_734.json";
	if(index == 735) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_735.json";
	if(index == 736) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_736.json";
	if(index == 737) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_737.json";
	if(index == 738) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_738.json";
	if(index == 739) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_739.json";
	if(index == 740) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_740.json";
	if(index == 741) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_741.json";
	if(index == 742) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_742.json";
	if(index == 743) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_743.json";
	if(index == 744) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_744.json";
	if(index == 745) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_745.json";
	if(index == 746) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_746.json";
	if(index == 747) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_747.json";
	if(index == 748) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_748.json";
	if(index == 749) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_749.json";
	if(index == 750) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_750.json";
	if(index == 751) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_751.json";
	if(index == 752) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_752.json";
	if(index == 753) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_753.json";
	if(index == 754) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_754.json";
	if(index == 755) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_755.json";
	if(index == 756) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_756.json";
	if(index == 757) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_757.json";
	if(index == 758) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_758.json";
	if(index == 759) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_759.json";
	if(index == 760) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_760.json";
	if(index == 761) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_761.json";
	if(index == 762) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_762.json";
	if(index == 763) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_763.json";
	if(index == 764) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_764.json";
	if(index == 765) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_765.json";
	if(index == 766) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_766.json";
	if(index == 767) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_767.json";
	if(index == 768) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_768.json";
	if(index == 769) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_769.json";
	if(index == 770) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_770.json";
	if(index == 771) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_771.json";
	if(index == 772) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_772.json";
	if(index == 773) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_773.json";
	if(index == 774) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_774.json";
	if(index == 775) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_775.json";
	if(index == 776) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_776.json";
	if(index == 777) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_777.json";
	if(index == 778) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_778.json";
	if(index == 779) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_779.json";
	if(index == 780) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_780.json";
	if(index == 781) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_781.json";
	if(index == 782) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_782.json";
	if(index == 783) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_783.json";
	if(index == 784) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_784.json";
	if(index == 785) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_785.json";
	if(index == 786) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_786.json";
	if(index == 787) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_787.json";
	if(index == 788) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_788.json";
	if(index == 789) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_789.json";
	if(index == 790) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_790.json";
	if(index == 791) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_791.json";
	if(index == 792) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_792.json";
	if(index == 793) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_793.json";
	if(index == 794) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_794.json";
	if(index == 795) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_795.json";
	if(index == 796) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_796.json";
	if(index == 797) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_797.json";
	if(index == 798) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_798.json";
	if(index == 799) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_799.json";
	if(index == 800) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_800.json";
	if(index == 801) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_801.json";
	if(index == 802) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_802.json";
	if(index == 803) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_803.json";
	if(index == 804) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_804.json";
	if(index == 805) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_805.json";
	if(index == 806) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_806.json";
	if(index == 807) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_807.json";
	if(index == 808) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_808.json";
	if(index == 809) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_809.json";
	if(index == 810) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_810.json";
	if(index == 811) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_811.json";
	if(index == 812) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_812.json";
	if(index == 813) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_813.json";
	if(index == 814) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_814.json";
	if(index == 815) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_815.json";
	if(index == 816) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_816.json";
	if(index == 817) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_817.json";
	if(index == 818) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_818.json";
	if(index == 819) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_819.json";
	if(index == 820) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_820.json";
	if(index == 821) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_821.json";
	if(index == 822) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_822.json";
	if(index == 823) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_823.json";
	if(index == 824) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_824.json";
	if(index == 825) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_825.json";
	if(index == 826) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_826.json";
	if(index == 827) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_827.json";
	if(index == 828) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_828.json";
	if(index == 829) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_829.json";
	if(index == 830) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_830.json";
	if(index == 831) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_831.json";
	if(index == 832) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_832.json";
	if(index == 833) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_833.json";
	if(index == 834) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_834.json";
	if(index == 835) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_835.json";
	if(index == 836) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_836.json";
	if(index == 837) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_837.json";
	if(index == 838) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_838.json";
	if(index == 839) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_839.json";
	if(index == 840) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_840.json";
	if(index == 841) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_841.json";
	if(index == 842) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_842.json";
	if(index == 843) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_843.json";
	if(index == 844) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_844.json";
	if(index == 845) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_845.json";
	if(index == 846) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_846.json";
	if(index == 847) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_847.json";
	if(index == 848) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_848.json";
	if(index == 849) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_849.json";
	if(index == 850) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_850.json";
	if(index == 851) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_851.json";
	if(index == 852) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_852.json";
	if(index == 853) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_853.json";
	if(index == 854) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_854.json";
	if(index == 855) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_855.json";
	if(index == 856) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_856.json";
	if(index == 857) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_857.json";
	if(index == 858) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_858.json";
	if(index == 859) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_859.json";
	if(index == 860) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_860.json";
	if(index == 861) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_861.json";
	if(index == 862) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_862.json";
	if(index == 863) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_863.json";
	if(index == 864) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_864.json";
	if(index == 865) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_865.json";
	if(index == 866) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_866.json";
	if(index == 867) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_867.json";
	if(index == 868) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_868.json";
	if(index == 869) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_869.json";
	if(index == 870) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_870.json";
	if(index == 871) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_871.json";
	if(index == 872) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_872.json";
	if(index == 873) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_873.json";
	if(index == 874) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_874.json";
	if(index == 875) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_875.json";
	if(index == 876) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_876.json";
	if(index == 877) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_877.json";
	if(index == 878) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_878.json";
	if(index == 879) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_879.json";
	if(index == 880) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_880.json";
	if(index == 881) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_881.json";
	if(index == 882) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_882.json";
	if(index == 883) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_883.json";
	if(index == 884) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_884.json";
	if(index == 885) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_885.json";
	if(index == 886) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_886.json";
	if(index == 887) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_887.json";
	if(index == 888) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_888.json";
	if(index == 889) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_889.json";
	if(index == 890) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_890.json";
	if(index == 891) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_891.json";
	if(index == 892) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_892.json";
	if(index == 893) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_893.json";
	if(index == 894) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_894.json";
	if(index == 895) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_895.json";
	if(index == 896) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_896.json";
	if(index == 897) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_897.json";
	if(index == 898) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_898.json";
	if(index == 899) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_899.json";
	if(index == 900) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_900.json";
	if(index == 901) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_901.json";
	if(index == 902) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_902.json";
	if(index == 903) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_903.json";
	if(index == 904) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_904.json";
	if(index == 905) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_905.json";
	if(index == 906) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_906.json";
	if(index == 907) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_907.json";
	if(index == 908) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_908.json";
	if(index == 909) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_909.json";
	if(index == 910) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_910.json";
	if(index == 911) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_911.json";
	if(index == 912) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_912.json";
	if(index == 913) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_913.json";
	if(index == 914) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_914.json";
	if(index == 915) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_915.json";
	if(index == 916) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_916.json";
	if(index == 917) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_917.json";
	if(index == 918) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_918.json";
	if(index == 919) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_919.json";
	if(index == 920) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_920.json";
	if(index == 921) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_921.json";
	if(index == 922) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_922.json";
	if(index == 923) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_923.json";
	if(index == 924) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_924.json";
	if(index == 925) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_925.json";
	if(index == 926) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_926.json";
	if(index == 927) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_927.json";
	if(index == 928) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_928.json";
	if(index == 929) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_929.json";
	if(index == 930) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_930.json";
	if(index == 931) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_931.json";
	if(index == 932) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_932.json";
	if(index == 933) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_933.json";
	if(index == 934) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_934.json";
	if(index == 935) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_935.json";
	if(index == 936) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_936.json";
	if(index == 937) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_937.json";
	if(index == 938) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_938.json";
	if(index == 939) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_939.json";
	if(index == 940) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_940.json";
	if(index == 941) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_941.json";
	if(index == 942) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_942.json";
	if(index == 943) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_943.json";
	if(index == 944) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_944.json";
	if(index == 945) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_945.json";
	if(index == 946) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_946.json";
	if(index == 947) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_947.json";
	if(index == 948) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_948.json";
	if(index == 949) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_949.json";
	if(index == 950) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_950.json";
	if(index == 951) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_951.json";
	if(index == 952) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_952.json";
	if(index == 953) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_953.json";
	if(index == 954) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_954.json";
	if(index == 955) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_955.json";
	if(index == 956) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_956.json";
	if(index == 957) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_957.json";
	if(index == 958) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_958.json";
	if(index == 959) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_959.json";
	if(index == 960) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_960.json";
	if(index == 961) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_961.json";
	if(index == 962) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_962.json";
	if(index == 963) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_963.json";
	if(index == 964) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_964.json";
	if(index == 965) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_965.json";
	if(index == 966) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_966.json";
	if(index == 967) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_967.json";
	if(index == 968) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_968.json";
	if(index == 969) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_969.json";
	if(index == 970) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_970.json";
	if(index == 971) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_971.json";
	if(index == 972) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_972.json";
	if(index == 973) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_973.json";
	if(index == 974) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_974.json";
	if(index == 975) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_975.json";
	if(index == 976) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_976.json";
	if(index == 977) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_977.json";
	if(index == 978) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_978.json";
	if(index == 979) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_979.json";
	if(index == 980) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_980.json";
	if(index == 981) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_981.json";
	if(index == 982) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_982.json";
	if(index == 983) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_983.json";
	if(index == 984) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_984.json";
	if(index == 985) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_985.json";
	if(index == 986) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_986.json";
	if(index == 987) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_987.json";
	if(index == 988) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_988.json";
	if(index == 989) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_989.json";
	if(index == 990) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_990.json";
	if(index == 991) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_991.json";
	if(index == 992) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_992.json";
	if(index == 993) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_993.json";
	if(index == 994) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_994.json";
	if(index == 995) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_995.json";
	if(index == 996) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_996.json";
	if(index == 997) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_997.json";
	if(index == 998) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_998.json";
	if(index == 999) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_999.json";
	if(index == 1000) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_1000.json";
	if(index == 1001) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_1001.json";
	if(index == 1002) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_1002.json";
	if(index == 1003) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_1003.json";
	if(index == 1004) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_1004.json";
	if(index == 1005) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_1005.json";
	if(index == 1006) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_1006.json";
	if(index == 1007) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_1007.json";
	if(index == 1008) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_1008.json";
	if(index == 1009) return "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/mandelbrot_nums_1009.json";
  return "";
}
