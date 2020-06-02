#include <iostream>
#include <functional>
#include <random>
#include <time.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <stdlib.h>


using namespace std;

class Particle
{
	public:
		float X, Y;
		float speedX, speedY;
};

class Local_Min
{
	public:
		float localX, localY;
		float localMin = 100000;
};

class Global_Min
{
    public:
		float globalX, globalY;
		float globalMin = 100000;
};

class Window
{
	public:
		float XStart = 0;
		float YStart = 0;
		float XEnd = 1000;
		float YEnd = 900;		
};

float F(float x, float y)
{
	//return 2*x + y;
    return x + y;
};

float RandomNumberGenerator(float seed, float min, float max) 
{
	thread_local static std::mt19937 rng(seed);
	thread_local std::uniform_real_distribution<float> urd;
	return urd(rng, decltype(urd)::param_type{min, max});
}

float SPO(int N, int NIteration, Window *window, float a, float b, float c, Particle *p, Local_Min *local, Global_Min *global)
{
	cout.precision(4);
	float _seed = time(NULL);	
	//cout<<_seed<<endl;
	
	
	//Giving starting point for each particle
	//cout<<"The initial points of Particles"<<endl;
	for(int i=0; i<N; i++)
	{
		p[i].X = RandomNumberGenerator(_seed, window->XStart, window->XEnd);
		p[i].Y = RandomNumberGenerator(_seed, window->XStart, window->XEnd);		
		//cout << p[i].X << " " << p[i].Y << endl;
	}
	
	//Giving initial speed for each Particle
	for(int i=0; i<N; i++)
	{
		p[i].speedX = RandomNumberGenerator(_seed, -10, 10);
		p[i].speedY = RandomNumberGenerator(_seed, -10, 10); 		
		//cout<<"Initial speeds of "<<i<<"th partical: "<<p[i].speedX<<" "<<p[i].speedY<<endl;
	}

	//Giving initial value for local and global min
    for(int i=0; i<N; i++)
	{
		local->localX = p[i].X;	
		local->localY = p[i].Y;	
		local->localMin = F(local->localX , local->localY);	
		
		if(local->localMin < global->globalMin)
		{
			global->globalX = local->localX;	
			global->globalY = local->localY;	
			global->globalMin = local->localMin;
		}
	}		
	

	//Starting our iterations
	for(int it=0; it<NIteration; it++)
	{
		float velocity;
		//cout<<"The new velocity of Particles"<<endl;
		
		//Updating velocities
		for(int i=0; i<N; i++)
		{
			velocity  =	a * p[i].speedX + 
						b * ((float) RandomNumberGenerator(_seed, 0, 1)/RAND_MAX) * (p[i].X - local->localX) +
						c * ((float) RandomNumberGenerator(_seed, 0, 1)/RAND_MAX) * (p[i].X - global->globalX);
						
			p[i].speedX = (velocity > 10) ? 10 : ((velocity < -10) ? -10 : velocity);
			
			velocity  =	a * p[i].speedY + 
						b * ((float) RandomNumberGenerator(_seed, 0, 1)/RAND_MAX) * (p[i].Y - local->localY) +
						c * ((float) RandomNumberGenerator(_seed, 0, 1)/RAND_MAX) * (p[i].Y - global->globalY);
			
			p[i].speedY = (velocity > 10) ? 10 : ((velocity < -10) ? -10 : velocity);
			//cout<<p[i].speedX <<" "<<p[i].speedY<<endl; 
		}
		
		//cout<<"The new positions of the points of Particles"<<endl;
		
		//Updating positions
		for(int i=0; i<N; i++)
		{			
			p[i].X = p[i].X + p[i].speedX;
			p[i].Y = p[i].Y + p[i].speedY;
						
			if(p[i].X < window->XStart)
				p[i].X = window->XStart + (window->XStart - p[i].X);
			if(p[i].X > window->XEnd)
				p[i].X = window->XEnd - (p[i].X - window->XEnd);
			
			if(p[i].Y < window->YStart)
				p[i].Y = window->YStart + (window->YStart - p[i].Y);
			if(p[i].Y > window->YEnd)
				p[i].Y = window->YEnd - (p[i].Y - window->YEnd);
			
			//cout<<p[i].X<<" "<<p[i].Y<<endl;
			
			float func = F(p[i].X, p[i].Y);
			
			//Finding localMin and GlobalMin
			if(func < local->localMin)
			{
				local->localX = p[i].X;	
				local->localY = p[i].Y;	
				local->localMin = func;	
				
				if(local->localMin < global->globalMin)
				{
					global->globalX = local->localX;	
					global->globalY = local->localY;	
					global->globalMin = local->localMin;
				}
			}
			
		}
	}
		
return global->globalMin;

};


int main(int argc, char * argv[])
{
	int N = 100000;					//number of particles
	int NIteration = 10000;		//number of iterations
	float a = 0.1;
	float b = 0.2;
	float c = 0.1;
	

	if(argc == 6)
	{	
		N = atoi(argv[1]);			//number of particles
		NIteration = atoi(argv[2]);	//number of iterations
		a = atoi(argv[3]);
		b = atoi(argv[4]);
		c = atoi(argv[5]);
	}
    else if(argc > 1)
	{
		std::cerr << "use: " << argv[0]  
				<< " N NIteration a b c\n";
        return -1;
    }

	Particle 	*p		 = new Particle[N];
	Local_Min 	*local	 = new Local_Min;
	Global_Min 	*global	 = new Global_Min;
	Window		*window  = new Window;
	
	//start program
	auto start = std::chrono::high_resolution_clock::now();
	float result =  SPO(N, NIteration, window, a, b, c, p, local, global);
	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	auto usec = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	//end of program
	
	std::cout << "The result of SOP is: " << result << endl<<endl;;
	std::cout<<"Time for computing is: "<<usec<<" microseconds"<<std::endl;

	return 0;
}