

#include <iostream>
#include "application.h"

using namespace ts;


void Application::init()
{
	std::cout << m_args;
	std::cin.get();
}