#include "Penrose.h"
#include <iostream>

/**
 * Constructor of Penrose Class
 *
 * @param _loops: The number of times for deflate tringles, MAX_RECOMENDED = 8
 * @param _origin: First coordinate for do a triangle
 * @param _degree: Just 36 degrees or 108 degrees
 * @param _height: The height of the tringle
 *
 */
Penrose::Penrose( int _loops, Coordinate _origin, int _degree, float _height ){
	loops = _loops;
	int totalTriangles = 360 / _degree;

	Coordinate p = Coordinate( _height, 0 );
	for( int i = 0; i < totalTriangles; i++ ){
		Coordinate temp = Coordinate::RotatePoint( _origin, _degree, p );
		//Coordinate temp = Coordinate::RotatePoint3D( _origin, _degree, p, "YZ");
		if( i % 2 == 0 ){
			triangles.push_back( Triangle( _origin, p, temp, 0 ) );
		} else{
			triangles.push_back( Triangle( _origin, temp, p, 0 ) );
		}
		p = temp;
	}

	NumTriangles = triangles.size();
}

/*
 * Clear the triangles vector
 */
Penrose::~Penrose(){
	triangles.clear();
}

void Penrose::execute(){
	for( int i = 0; i < loops; i++ )
		triangles = deflate();

	NumTriangles = triangles.size();
}

/**
 * Create the deflate around the principal triangle
 */
std::vector<Triangle> Penrose::deflate(){
	std::vector<Triangle> temp;

	for( const Triangle &t : triangles ){
		if( t.type == 2 ){

			// B + ( ( A - B) / PHI )
			Coordinate Q = Coordinate::sum( t.b, Coordinate::divide( Coordinate::diff( t.a, t.b ), PHI ) );
			// B + ( ( C - B) / PHI )
			Coordinate R = Coordinate::sum( t.b, Coordinate::divide( Coordinate::diff( t.c, t.b ), PHI ) );

			temp.push_back( Triangle( R, t.c, t.a, 1 ) );
			temp.push_back( Triangle( Q, R, t.b, 1 ) );
			temp.push_back( Triangle( R, Q, t.a, 0 ) );

		} else if( t.type == 1 ){

			Coordinate P = Coordinate::sum( t.a, Coordinate::divide( Coordinate::diff( t.b, t.a ), PHI ) );

			temp.push_back( Triangle( t.c, P, t.b, 0 ) );
			temp.push_back( Triangle( P, t.c, t.a, 1 ) );

		}
	}

	return temp;
}

void Penrose::DoIt3D(){
	std::vector<Triangle> temp = DoIT3D();

	for( const Triangle &t : temp ){
		triangles.push_back( t );

		NumTriangles++;
	}
}

std::vector<Triangle> Penrose::DoIT3D(){
	std::vector<Triangle> temp;

	for( Triangle &t : triangles ){
		glm::vec3 origin = glm::vec3( t.a.x, t.a.y, t.a.z );
		glm::vec3 Normalized_Vector = t.GetNormalOfTriangle();
		glm::vec3 top_point = origin + ( Normalized_Vector );

		temp.push_back(
			Triangle(
				Coordinate( top_point.x, top_point.y, top_point.z ),
				t.b,
				t.a,
				t.type - 1
			)
		);
		temp.push_back(
			Triangle(
				Coordinate( top_point.x, top_point.y, top_point.z ),
				t.a,
				t.c,
				t.type - 1
			)
		);
		temp.push_back(
			Triangle(
				Coordinate( top_point.x, top_point.y, top_point.z ),
				t.c,
				t.b,
				t.type - 1
			)
		);
	}

	return temp;
}

float *Penrose::GetVertices(){
	int vectorSize = NumTriangles * 9;
	float *vertices = new float[ vectorSize ];

	int i = 0;
	for( Triangle t : triangles ){
		float *triangleVertices = t.getTriangleCoordinates();

		for( int j = 0; j < 9; j++ ){
			vertices[ i ] = triangleVertices[ j ];
			i++;
		}
	}

	return vertices;
}

float *Penrose::GetVerticesWithColors(){
	int vectorSize = NumTriangles * 18;
	float *vertices = new float[ vectorSize ];

	int i = 0;
	for( Triangle t : triangles ){
		float *triangleVertices = t.getTriangleCoordinatesWithColors();

		for( int j = 0; j < 18; j++ ){
			vertices[ i ] = triangleVertices[ j ];
			i++;
		}

	}

	return vertices;
}

float *Penrose::GetVerticesWithTextureCoords(){
	int vectorSize = NumTriangles * 18;
	float *vertices = new float[ vectorSize ];

	int i = 0;
	for( Triangle t : triangles ){
		float *triangleVertices = t.getTriangleCoordinatesWithTexCoords();

		for( int j = 0; j < 18; j++ ){
			vertices[ i ] = triangleVertices[ j ];
			i++;
		}

	}

	return vertices;
}

float *Penrose::GetVerticesWithColorsAndTextureCoords(){
	int vectorSize = NumTriangles * 27;
	float *vertices = new float[ vectorSize ];

	int i = 0;
	for( Triangle t : triangles ){
		float *triangleVertices = t.getTriangleCoordinatesWithColorsAndTexCoords();

		for( int j = 0; j < 27; j++ ){
			vertices[ i ] = triangleVertices[ j ];
			i++;
		}

	}

	return vertices;
}

float *Penrose::GetVerticesWithColorsTexCoordsAndNormalLight(){
	int vectorSize = NumTriangles * 36;
	float *vertices = new float[ vectorSize ];

	int i = 0;
	for( Triangle t : triangles ){
		float *triangleVertices = t.getTriangleCoordinatesWithColorsTexCoordsAndNormalLight();

		for( int j = 0; j < 36; j++ ){
			vertices[ i ] = triangleVertices[ j ];
			i++;
		}

	}

	return vertices;
}
