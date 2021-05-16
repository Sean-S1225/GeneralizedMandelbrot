//This code is used to visualize each frame of the mandelbrot set
//created by the C program. It finds each .json file in the folder,
//then for each frame stored in the .json file, renders it,
//saves it, then deletes the .json file at the end.
//The reason why the algorithm runs twice is because the splitting
//python file creates file that look like mandelbrot_nums_[num]_0.json and
//mandelbrot_nums_[num]_1.json .

import java.awt.event.MouseEvent;
import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;

import processing.core.PApplet;
import processing.core.PImage;
import processing.data.JSONArray;
import processing.data.JSONObject;

class Alg extends Thread{
	public void run() {
		
	}
}

public class mandelbrot extends PApplet{
	public static void main(String[] args) {
		PApplet.main("mandelbrot");
	}
	
	int width, height, iterations;
	float nums[][];
	JSONObject json;
	JSONArray arr;
	
	public void settings() {
		size(900, 900);
	}
	
	public void setup() {
		background(0);
		colorMode(HSB, 255);
//		colorMode(RGB);
		
		File folder = new File("/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/");
		String[] filesTemp = folder.list();
		Arrays.sort(filesTemp);
		ArrayList<String> realFiles = new ArrayList<String>();
		
		for (String file : filesTemp)
        {
            if(file.substring(file.length() - 5).equals(".json")) {
            	realFiles.add(file);
            }
        }
				
		String[] files = new String[realFiles.size()];
		
		for (int i = 0; i < realFiles.size(); i++)
        {
            files[i] = realFiles.get(i);
        }

//		for(int x = 0; x < files.length / 2; x++) {
		for(int x = 0; x < files.length; x++) {
			String name = files[2 * x].split("_")[2];
			json = loadJSONObject("files/mandelbrot_nums_" + name + "_0.json");
			File file = new File("files/mandelbrot_nums_" + name + "_0.json");
		
			width = json.getInt("width");
			height = json.getInt("height");
			iterations = json.getInt("iterations");
		
			nums = new float[iterations][width*height];
		
			arr = json.getJSONArray("nums");
			for(int i = 0; i < iterations; i++) {
				nums[i] = arr.getJSONArray(i).getFloatArray();
			}
//		
			for(int i = 0; i < nums.length; i++) {
				background(0);
				for(int j = 0; j < nums[i].length; j++) {
					if(!Double.isNaN(nums[i][j])) {
						stroke(nums[i][j], 255, 255);
//						stroke(0, 0, (float) nums[i][j]);
					}else {
//						stroke(255, 255, 0);
						stroke(0, 0, 0);
					}
					point((int)(j / height), j % height);
				}
			
				saveFrame("/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/imgs/" + files[2 * x].split("_")[2] + "_0_" + Integer.toString(i) + ".png");
				System.out.println(name + ", " + (i + 1) + "/" + iterations);
			}
			
			json = new JSONObject();
			arr = new JSONArray();
			nums = new float[iterations][width*height];
			
			file.delete();
			
			System.gc();
						
			json = loadJSONObject("files/mandelbrot_nums_" + name + "_1.json");
			file = new File("files/mandelbrot_nums_" + name + "_1.json");
		
			width = json.getInt("width");
			height = json.getInt("height");
			iterations = json.getInt("iterations");
			nums = new float[iterations][width*height];
				
			arr = json.getJSONArray("nums");
			for(int i = 0; i < iterations; i++) {
				nums[i] = arr.getJSONArray(i).getFloatArray();
			}
		
			for(int i = 0; i < nums.length; i++) {
				background(0);
				for(int j = 0; j < nums[i].length; j++) {
					if(!Double.isNaN(nums[i][j])) {
						stroke(nums[i][j], 255, 255);
//						stroke(0, 0, (float) nums[i][j]);
					}else {
//						stroke(255, 255, 0);
						stroke(0, 0, 0);
					}
					point((int)(j / height), j % height);
				}
			
				saveFrame("/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/imgs/" + files[2 * x].split("_")[2] + "_1_" + Integer.toString(i) + ".png");
				System.out.println(name + ", " + (i + 1) + "/" + iterations);
			}
			
			json = new JSONObject();
			arr = new JSONArray();
			nums = new float[iterations][width*height];
			
			file.delete();
			
			System.gc();
		}
		
		print("done");
	}
}
