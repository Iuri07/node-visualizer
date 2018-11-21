#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <tuple>

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Loads individual image as texture
SDL_Texture* loadTexture( std::string path );

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

std::ifstream input("./points/points-1000.txt");

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ){
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) ){
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "Node Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,\
		SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL ){
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
			if( gRenderer == NULL ){
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) ){
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia(){
	//Loading success flag
	bool success = true;

	//Nothing to load
	return success;
}

void close(){
	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

SDL_Texture* loadTexture( std::string path ){
	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL ){
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else{
		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL ){
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	return newTexture;
}

void draw_border(float center_x, float center_y, int radius, SDL_Color color){
    SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, color.a);

    for (int w = 0; w < 360; w++){
		int dx = radius*cosf(w); // horizontal offset
		int dy = radius*sinf(w); // vertical offset
		if((dx*dx + dy*dy) <= (radius*radius)){
			SDL_RenderDrawPoint(gRenderer, center_x + dx, center_y + dy);
		}
    }
}

void draw_circle(float center_x, float center_y, int radius, SDL_Color color, int border_width){
	SDL_Color border = {0, 0, 0};
    SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, color.a);

    for (int w = 0; w < radius - border_width; w++){
		draw_border(center_x, center_y,w, color);
    }
    for (int w = radius - border_width; w < radius; w++){
		draw_border(center_x, center_y,w, border);
	}
}

void draw_line(float start_x, float start_y, float end_x, float end_y, SDL_Color color){
    SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(gRenderer, start_x, start_y, end_x, end_y);
}

float order_up(float n) {
    float order = ceil(log(n) / log(10) + 0.000000001); 
    return pow(10,order);
}

float map( float x, float in_min, float in_max,float out_min, float out_max){
  return ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

std::vector<std::tuple<float, float>> normalize(std::vector<std::tuple<float, float>> node_list){
	float max = 0.0;
	for(int i = 0; i < node_list.size(); ++i) {
		if(std::get<0>(node_list[i]) > max){
			max = std::get<0>(node_list[i]);
		}
		if (std::get<1>(node_list[i]) > max){
			max = std::get<1>(node_list[i]);
		}
	}

	max = order_up(max);
	float norm_x, norm_y;
	std::vector<std::tuple<float, float>> normalized_nodes;
	for(int i = 0 ; i< node_list.size(); ++i){
		norm_x = map(std::get<0>(node_list[i]), 0, max, 10, SCREEN_HEIGHT - 10);
		norm_y = map(std::get<1>(node_list[i]), 0, max, SCREEN_HEIGHT - 10, 10);
		normalized_nodes.push_back(std::make_tuple(norm_x, norm_y));
	}
	return normalized_nodes;
}



std::vector<std::tuple<float, float>> parse_nodes(){
	std::vector<std::tuple<float, float>> nodes;
	for( std::string line; getline( input, line ); ){	
    	int x, y;
		if(!(input >> x >> y)) {break;}
		else{
			nodes.push_back(std::make_tuple(x, y));
		}
	}
	
	std::vector<std::tuple<float, float>> normalized_nodes = normalize(nodes);
	return normalized_nodes;

}

int main(int argc, char* argv){
	//Start up SDL and create window
	if( !init() ){
		printf( "Failed to initialize!\n" );
	}
	else{
		//Load media
		if( !loadMedia() ){
			printf( "Failed to load media!\n" );
		}
		else{	
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//While application is running
			std::vector<std::tuple<float, float>> nodes = parse_nodes();

			//Clear screen
			SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
			SDL_RenderClear( gRenderer );
			
			for(int i = 0; i < nodes.size(); ++i){
				int tone = rand() % 200;
				SDL_Color line_color = {200, 0, 0};
				SDL_Color circle_color = {tone, tone, tone};
				if(i+1 < nodes.size()){
					draw_line(std::get<0>(nodes[i]), std::get<1>(nodes[i]), std::get<0>(nodes[i+1]), std::get<1>(nodes[i+1]), line_color);
				}else{
					draw_line(std::get<0>(nodes[i]), std::get<1>(nodes[i]), std::get<0>(nodes[0]), std::get<1>(nodes[0]), line_color);
				}
				draw_circle(std::get<0>(nodes[i]), std::get<1>(nodes[i]), 4, circle_color, 1);
			}
			//Update screen
			SDL_RenderPresent( gRenderer );
			while( !quit ){
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 ){
					//User requests quit
					if( e.type == SDL_QUIT ){
						std::cout << "Clean shutdown.." << std::endl; 
						quit = true;
					}
				}
				usleep(400000);

				
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}