// Termm--Fall 2020

#include "Image.hpp"

#include <iostream>
#include <cstring>
#include <lodepng/lodepng.h>

const uint Image::m_colorComponents = 3; // Red, blue, green

//---------------------------------------------------------------------------------------
Image::Image()
  : m_width(0),
    m_height(0),
    m_data(0)
{
}

//---------------------------------------------------------------------------------------
Image::Image(
		uint width,
		uint height
)
  : m_width(width),
    m_height(height)
{
	size_t numElements = m_width * m_height * m_colorComponents;
	m_data = new double[numElements];
	memset(m_data, 0, numElements*sizeof(double));
}

//---------------------------------------------------------------------------------------
Image::Image(const Image & other)
  : m_width(other.m_width),
    m_height(other.m_height),
    m_data(other.m_data ? new double[m_width * m_height * m_colorComponents] : 0)
{
  if (m_data) {
    std::memcpy(m_data, other.m_data,
                m_width * m_height * m_colorComponents * sizeof(double));
  }
}

//---------------------------------------------------------------------------------------
Image::~Image()
{
  delete [] m_data;
}

//---------------------------------------------------------------------------------------
Image & Image::operator=(const Image& other)
{
  delete [] m_data;
  
  m_width = other.m_width;
  m_height = other.m_height;
  m_data = (other.m_data ? new double[m_width * m_height * m_colorComponents] : 0);

  if (m_data) {
    std::memcpy(m_data,
                other.m_data,
                m_width * m_height * m_colorComponents * sizeof(double)
    );
  }
  
  return *this;
}

//---------------------------------------------------------------------------------------
uint Image::width() const
{
  return m_width;
}

//---------------------------------------------------------------------------------------
uint Image::height() const
{
  return m_height;
}

//---------------------------------------------------------------------------------------
double Image::operator()(uint x, uint y, uint i) const
{
  return m_data[m_colorComponents * (m_width * y + x) + i];
}

//---------------------------------------------------------------------------------------
double & Image::operator()(uint x, uint y, uint i)
{
  return m_data[m_colorComponents * (m_width * y + x) + i];
}

//---------------------------------------------------------------------------------------
static double clamp(double x, double a, double b)
{
	return x < a ? a : (x > b ? b : x);
}

//---------------------------------------------------------------------------------------
bool Image::savePng(const std::string & filename) const
{
	std::vector<unsigned char> image;

	image.resize(m_width * m_height * m_colorComponents);

	double color;
	for (uint y(0); y < m_height; y++) {
		for (uint x(0); x < m_width; x++) {
			for (uint i(0); i < m_colorComponents; ++i) {
				color = m_data[m_colorComponents * (m_width * y + x) + i];
				color = clamp(color, 0.0, 1.0);
				image[m_colorComponents * (m_width * y + x) + i] = (unsigned char)(255 * color);
			}
		}
	}

	// Encode the image
	unsigned error = lodepng::encode(filename, image, m_width, m_height, LCT_RGB);

	if(error) {
		std::cerr << "encoder error " << error << ": " << lodepng_error_text(error)
				<< std::endl;
	}

	return true;
}

bool Image::loadPng(const std::string& filename){
  std::vector<unsigned char> image;
  uint width, height;

  unsigned error = lodepng::decode(image, width, height, filename, LCT_RGB);

	if(error) {
		std::cerr << "decoder error " << error << ": " << lodepng_error_text(error)
				<< std::endl;
	}

  setWidth(width);
  setHeight(height);
  size_t numElements = m_width * m_height * m_colorComponents;
  setData(image, numElements);

	return true;
}

glm::vec3 Image::operator()(uint x, uint y) const
{
  if(x >= m_width) { 
    x = x % m_width; 
  }else if(x < 0) { 
    x = (-x) % m_width;
    x = m_width - x;
  }

  if(y >= m_height) { 
    y = y % m_height; 
  }else if(y < 0) { 
    y = (-y) % m_height;
    y = m_height - y;
  }
  return glm::vec3((*this)(x,y,0),(*this)(x,y,1),(*this)(x,y,2));
}

bool Image::empty() const {
  return (m_data == NULL);
}

// u,v of range (0, 1)
glm::vec3 Image::get_color(double u, double v) const {
  double u_pos = u * (double) m_width;
  double v_pos = v * (double) m_height;

	int ui = (int) floor(u_pos);
	int vi = (int) floor(v_pos);

  double u_weight = u_pos - ui;
  double v_weight = v_pos - vi;

  // Bilinear interpolation between pixel values, basically a weighted average of the sourrounu_posng pixels
  // using the barycentric cooru_posnates u_weight, v_weight
  return (1-u_weight)*(1-v_weight) * (*this)(ui, vi) +
    (1-u_weight)*(v_weight) * (*this)(ui, vi + 1) +
    (u_weight)*(1-v_weight) * (*this)(ui + 1, vi) +
    (u_weight)*(v_weight) * (*this)(ui + 1, vi + 1);
}

//---------------------------------------------------------------------------------------
const double * Image::data() const
{
  return m_data;
}

//---------------------------------------------------------------------------------------
double * Image::data()
{
  return m_data;
}
