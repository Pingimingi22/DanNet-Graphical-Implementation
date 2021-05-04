#include "Example.h"
#include <iostream>



int main()
{
	Example program;

	while (program.IsRunning())
	{
		program.Update();
		program.Render();
	}
	return 0;
}