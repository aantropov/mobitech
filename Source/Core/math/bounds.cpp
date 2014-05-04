#include "bounds.h"
#include "vec3.h"
#include "vec4.h"
#include "quat.h"
#include "mat2.h"
#include "mat3.h"
#include "mat4.h"
#include "../Renderer.h"

void AABB:: Calculate(VertexBuffer* p, mat4 model)
{
    for(int i = 0; i < p->GetNum(); i++)
    {      
        vec3 point = model * ((Vertex*)p->GetPointer())[i].pos;
        if(point.x < top_left.x)
			top_left.x = point.x;
		if(point.x > right_down.x)
			right_down.x = point.x;
		if(point.y < top_left.y)
			top_left.y = point.y;
		if(point.y > right_down.y)
			right_down.y = point.y;
    }
}

inline double Determinant(double a11, double a12, double a21, double a22)
{
    return a11 * a22 - a12 * a21;
}

bool IntersectLine(vec3* a1, vec3* a2, vec3* b1, vec3* b2,vec3* c)
{
    double ady = a1->y - a2->y;
    double adx = a2->x - a1->x;
    double bdy = b1->y - b2->y;
    double bdx = b2->x - b1->x;
    double D = Determinant(ady, adx, bdy, bdx);

    if (fabs(D) < math_epsilon)
	    return true;

    double aC = adx * a1->y + ady * a1->x;
    double bC = bdx * b1->y + bdy * b1->x;
    double DX = Determinant(aC, adx, bC, bdx);
    double DY = Determinant(ady, aC, bdy, bC);
    
    c->x = DX / D;
    c->y = DY / D;
  
    return false;
}

bool IntersectConvexShapePoint(VertexBuffer* buffer, mat4 model, vec3* point)
{
    vec3 model_space_point = inverse(model) * (*point);
    vec3 a = ((Vertex*)buffer->GetPointer())[0].pos - model_space_point;
	vec3 b;
	
    double summ = 0;
	for(int i = 1; i< buffer->GetNum() ; i++)
    {
		b = ((Vertex*)buffer->GetPointer())[i] - model_space_point;
        summ += dot(a,b);
		a = b;	
	}

	return summ < 360;
}

bool IntersectAABB(AABB *a, AABB *b)
{
    return a->right_down.x < b->top_left.x || b->right_down.x < a->top_left.x || a->right_down.y < b->top_left.y || b->right_down.y < a->top_left.y;
}

// [res.x,res.y] - interval of projection Poly to Vector
vec3 Projection(vector<vec3> &a, vec3 b){
	vec3 res;
    res.y = res.x = projection(a[0], b);
    for(int i = 1; i < a.size() ; i++){
		double temp = projection(a[i], b);
		if(temp < res.x)
			res.x = temp;
        if(temp > res.y)
			res.y = temp;
	}
	return res;
}

//Separeting Axis Theorem
bool IntersectConvexShape(VertexBuffer* a, mat4 model_a, VertexBuffer* b, mat4 model_b, vec3 &contact, vec3 &c, double &d)
{
	// potential separating axis
	vector<vec3> psa; 

	vector<vec3> transformed_a;
    for(int i = 0; i< a->GetNum(); i++)
        transformed_a.push_back(model_a * ((Vertex*)a->GetPointer())[i].pos);
	
    vector<vec3> transformed_b;
    for(int i = 0; i< b->GetNum(); i++)
        transformed_b.push_back(model_b * ((Vertex*)b->GetPointer())[i].pos);
	
    for(int i = 0; i < transformed_a.size()- 1; i++)
    {
		vec3 temp = transformed_a[i+1] - transformed_a[i];
        psa.push_back(GLRotationZ(90.0f) * temp);
	}

    vec3 last = transformed_a[0] - transformed_a[transformed_a.size()-1];
	psa.push_back(GLRotationZ(90.0f) * last);
	
    for(int i = 0; i < transformed_b.size()- 1; i++)
    {
		vec3 temp = transformed_b[i+1] - transformed_b[i];
        psa.push_back(GLRotationZ(90.0f) * temp);
	}

    last = transformed_b[0] - transformed_b[transformed_b.size()-1];
	psa.push_back(GLRotationZ(90.0f) * last * -1.0);

	//check axies
	int min_index = -1;
	double min = 0.0;
    double proj = 0.0;
    double p = 0.0;

    for(int i = 0; i < psa.size(); i++)
    {
		vec3 r1 = Projection(transformed_a, psa[i]);
		vec3 r2 = Projection(transformed_b, psa[i]);
		
		if(r1.y < r2.x)
			return false;
		
        if(r2.y < r1.x)
			return false;
		
        if(r2.x < r1.y && r1.y < r2.y)
        {
			if((min_index == -1 && r1.y - r2.x > 0) ||(min > r1.y - r2.x && r1.y - r2.x > 0))
            {
				min = r1.y - r2.x;
				min_index = i;
			}
		}

		if(r1.x < r2.y && r2.y < r1.y)
        {
			if((min_index == -1 && r2.y - r1.x < 0) ||(min > r2.y - r1.x && r2.y - r1.x < 0))
            {
				min = r2.y - r1.x;
				min_index = i;
			}
		}		
	}
	c = psa[min_index];
	d = fabs(min);

	//result vector, lenght = intersection 
    c *= d / length(c);

	bool a_intersect = false;
	vector<vec3> *intersected_obj = &transformed_a;
	vector<vec3> *intersect_obj = &transformed_b;

    if(min_index >= transformed_a.size())
    {
		a_intersect = true;
		intersected_obj = &transformed_b;
		intersect_obj   = &transformed_a;
		min_index -= transformed_a.size();
	}
	
	// intersect edge
	vec3 p1 , p2;
	if( min_index == intersected_obj->size() - 1)
    {
        p1 = (*intersected_obj)[intersected_obj->size()-1];	
        p2 = (*intersected_obj)[0];	
    }
    else
    {
        p1 = (*intersected_obj)[min_index];	
        p2 = (*intersected_obj)[min_index+1];	
    }

	// intersect point
	p2 = p2 - p1;
    double l = length(p2);
	
    for(int i = 0; i < intersect_obj->size(); i++)
    {
        vec3 temp = (*intersect_obj)[i] - p1;
        double temp_proj = projection(temp, p2);
        double dist = fabs(distance_to_line(temp, vec3(0.0f, 0.0f, 0.0f), p2));

        if(-math_epsilon <= temp_proj && temp_proj <= l + math_epsilon && 
            dist <= d + math_epsilon && dist >= d - math_epsilon)
            contact = (*intersect_obj)[i];
    }
    return true;
}