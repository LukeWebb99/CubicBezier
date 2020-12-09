#include <iostream>

#include <SFML/Graphics.hpp>

const int windowWidth = 600, windowHeight = 600;

sf::Vector2f HandleMousePosition(sf::Vector2i mousePos) {

    float x, y;
    
    if (mousePos.x >= windowWidth)
        x = windowWidth;
    else if (mousePos.x <= 0)
        x = 0;
    else
        x = mousePos.x;

    if (mousePos.y >= windowHeight)
        y = windowWidth;
    else if (mousePos.y <= 0)
        y = 0;
    else
        y = mousePos.y;

    return sf::Vector2f(x, y);
}

float NegToPos(float f) {
    return  f - (f * 2);
}

struct Point : sf::CircleShape //extend sf::circleShape class to include custom functions
{
    Point(sf::Vector2f position, float size, sf::Color colour) {
        this->setRadius(size); //set size
        this->setPosition(position); //set position
        this->setOrigin(sf::Vector2f(size, size)); // set origin
        this->setFillColor(colour); //set fill colour
    }

    void Draw(sf::RenderTarget& target) { //draw function
        target.draw(*this); // pass this object to the render target(main window)
    }

    bool Draggable(sf::Vector2f mousePos, bool mouseClicked) { //check if is being dragged

        sf::Vector2f diff = mousePos - this->getPosition(); //get differant between mouse pos and this points pos

        if (diff.x < 0) //if differance.x is negative value flipped
            diff.x = NegToPos(diff.x);

        if (diff.y < 0) //if differance.y is negative value flipped
            diff.y = NegToPos(diff.y);

        if (mouseClicked && diff.x <= this->getRadius() * 2 && diff.y <= this->getRadius() * 2) //if mouse is clicked within points radius
        {
            this->setPosition(mousePos); //set points pos to mouse pos
           return true; 
        }
        else {
            return false; 
        }
    }
   
    void CheckOverlap(Point* p) {
        if (this->getPosition() == p->getPosition()) {//if this point and other point have same position

            sf::Vector2f diff = p->getPosition() - sf::Vector2f(windowWidth/2, windowHeight/2); //get differance between this point and center of window
            sf::Vector2f t = sf::Vector2f(diff.x * 0.01, diff.y * 0.1); //shrink value by 0.1

            sf::Vector2f g = sf::Vector2f(this->getPosition() - sf::Vector2f(p->getRadius() * t.x * 1.1, p->getRadius() * t.y * 1.1));//calculate distance to move point 
            this->setPosition(g.x, g.y); //move point
        }
    }

};

sf::Vector2f GeneratePoints(float t, sf::Vector2f sPoint, sf::Vector2f sControl, sf::Vector2f ePoint, sf::Vector2f eControl) {
    //interpellate between points then return 
    return (std::powf(t, 3) * (ePoint + 3.f * (sControl - eControl) - sPoint) + 3.f *
            std::powf(t, 2) * (sPoint - 2.f * sControl + eControl) + 3.f * 
            std::powf(t, 1) * (sControl - sPoint) + sPoint);

}

std::vector<sf::Vector2f> CalcCubicBezier(sf::Vector2f sPoint, sf::Vector2f sControl, sf::Vector2f ePoint, sf::Vector2f eControl, int numOfSegments) {

    std::vector<sf::Vector2f> ret;
    if (!numOfSegments) // if numOfSegments == 0, exit
        return ret;

    ret.push_back(sPoint); // First point is fixed
    float p = 1.f / numOfSegments; // caluclate steps 't'
    float iter = p; //calculate amount to iterate per loop
    for (size_t i = 0; i < numOfSegments; i++, p += iter) { 
        ret.push_back(GeneratePoints(p, sPoint, sControl, ePoint, eControl)); // Generate all between
    }
    ret.push_back(ePoint); // Last point is fixed

    return ret; //return array

}

int main() {

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "window"); //create window
    bool isClicked = false; // bool for checking if left mouse button is down
   
    std::vector<Point> dots; //Array of Control points 
    dots.push_back(Point({ 20, 20 }, 4, sf::Color::White)); //Start
    dots.push_back(Point({ 20, 580 }, 4, sf::Color::Red)); //Start Control

    dots.push_back(Point({ 300, 300 }, 4, sf::Color::White)); //Middle
    dots.push_back(Point({ 300, 590 }, 4, sf::Color::Red)); //First Middle Control
    dots.push_back(Point({ 300, 10 }, 4, sf::Color::Red)); //Second Middle Control
    
    dots.push_back(Point({ 580, 580 }, 4, sf::Color::White)); //End 
    dots.push_back(Point({ 580, 20}, 4, sf::Color::Red)); //End Control
    
    std::vector<sf::Vector2f> points, temp; /*
                                            points: array of final points to be drawn
                                            temp: temp array of points to be added to points array
                                            */

    sf::VertexArray vertices(sf::LineStrip, 0); // a vertex array to create a line from after all points have been calculated 

    while (window.isOpen()) 
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) //hanfle window close 
                window.close(); //close window 

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) //handles left mouse pressed
                isClicked = true; //if left button on mouse is pressed toggle this bool

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) //handles left mouse relase
                isClicked = false; //if left button on mouse is relase reset this bool

        }

        window.clear(); //clear window

        for (size_t i = 0; i < dots.size(); i++) { //for all control points on screen

            for (size_t j = 1; j < dots.size(); j++) {//for all control points on screen +1
                if(i != j) //if I != J it is a differant point
                    dots[i].CheckOverlap(&dots[j]); //if two points overlap move one closer to the center
            }
            dots[i].Draggable(HandleMousePosition(sf::Mouse::getPosition(window)), isClicked); //check if control point is being dragged
            dots[i].Draw(window); //draw control point
               
        }

        points = CalcCubicBezier(dots[0].getPosition(), dots[1].getPosition(), dots[2].getPosition(), dots[3].getPosition(), 50); // uses CubicBezier algorithm to interpellate between the 4 control points
        temp   = CalcCubicBezier(dots[2].getPosition(), dots[4].getPosition(), dots[5].getPosition(), dots[6].getPosition(), 50); 
        points.insert(points.end(), temp.begin(), temp.end()); // add temp array to main points array

        for(size_t i = 0; i < points.size(); i++) //for all points
            vertices.append(sf::Vertex(points[i], sf::Color::White)); // pass all interpellated points to vertex array for drawing 

        window.draw(vertices); // draw vertices as linestrip

        window.display(); //update window

       points.clear(); //clear all arrays for next loop
       temp.clear();
       vertices.clear();

    }

    return 0;
}