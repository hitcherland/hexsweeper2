PGraphics pg;
float[] _j;
float innerRadius, outerRadius, _h;

void setup()
{
	int m = 4 * min( screen.width, screen.height );
	size( m, m );
	frameRate( 30 );
	background(10);
	//noLoop();
	outerRadius = 4; 
	strokeWeight( 8 );

	innerRadius = 0.5 * ( width - 8 ) / ( ( 2 * outerRadius + 1 ) * cos( PI / 6 ) );
	_h = 2 * innerRadius * cos( PI / 6 );
	_j = new float[ 2 ];
	_j[ 0 ] = innerRadius * cos( PI / 6 );
	_j[ 1 ] = 2 * _j[ 0 ] * cos( PI / 6 );
}

void drawHexagon( float x, float y, float radius ) {
	beginShape();
	for( int i=0; i<=6; i=i+1 ) {
		vertex(	x + radius * sin( i * PI / 3 ), y + radius * cos( i * PI / 3 ) )
	}
	endShape();
}

void drawHex( float x, float y ) {
	noFill();
	stroke( 0, 0, 255 );
	drawHexagon( x, y, innerRadius );
	//fill( 0 );
	//noStroke();
	//drawHexagon( x, y, innerRadius * 0.90 ); 
}

float[] mouseHex() {
	float[] output = new float[ 2 ];
	float x = mouseX;// - width / 2.0;
	float y = mouseY * screen.height - 0.5;
	output[ 1 ] = round( y * height / ( _j [ 1 ] * screen.height ) );
	output[ 0 ] = x;
	return output;
}

void mouseMoved() {
	println( mouseX + "," + mouseY + " -> " +mouseHex() );
}

void draw(){  
	int cx = width / 2;
	int cy = height / 2;
	float x,y;

	for( int h=-outerRadius; h<=outerRadius; h=h+1 ) {
		int H = abs( h );
		for( int j=-outerRadius; j<=outerRadius; j=j+1 ) {
			int J = abs( j );
			if( abs( h + j ) > outerRadius)
				continue;
			x = cx + h * _h + j * _j[ 0 ];
			y = cy + j * _j[ 1 ];
			drawHex( x, y );
		}
	}
}
