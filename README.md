# Ptr
Lightweight C++ smart pointer library

### Features
* Scoped Pointers and Reference Pointers
* Automatic memory managment (Memory is cleaned automatically)

### Usage
Usage is simple, declare the pointer that you wish to use like any other class, you will create a heap allocated object that can be used like a regular pointer. Scoped Pointers are unique, and cannot share a memory address. Reference Pointers can be used if you wish to have a heap allocated object, with multiple pointers pointing to it

### Examples
* Heap allocating using ScopedPtr
  
```c++
#include <iostream>
#include "Ptr.h"
//example structure
struct Vector2
{
  Vector2(float x, float y)
    : x(x), y(y)
  {
  }

  float x;
  float y;
};

int main()
{
  {
    //allocate using safe function
    Ptr::ScopedPtr<Vector2> ptr = Ptr::InitScopedPtr<Vector2>(5, 6);
    ptr->x = 7;
  
    std::cout << "Values: " << ptr->x << " " << ptr->y << std::endl;
    //will output Values: 7 6
    
    //allocate using constructor
    Ptr::ScopedPtr<int> ptr(new int(4));
    std::cout << *ptr << std::endl;
    //will output 4
    
  }//memory gets cleaned here.
  
  return 0;
}
```
  
* Heap allocating using RefPtr and ScopedPtr

```c++
#include <iostream>
#include "Ptr.h"

class Window
{
public:
  Window() { ... }
  ~Window() { ... }
};

class Renderer
{
public:
  Renderer() { ... }
  ~Renderer() { ... }
  
  void AssignWindow(const Ptr::RefPtr<Window>& _ptr) { ptr = _ptr; }
  
private:
  Ptr::RefPtr<Window> ptr;
};

int main()
{
  Ptr::RefPtr<Window> ptr = Ptr::InitRefPtr<Window>();
  
  {
    Ptr::ScopedPtr<Renderer> renderer(new Renderer());
    renderer->AssignWindow(ptr);
    
    std::cout << ptr.GetRefCount() << "\n";
    //output is 2
  } // memory allocated for renderer gets deleted here. Reference count also decreases
  
  std::cout << ptr.GetRefCount() << "\n";
  //output is 1
  
  return 0;
  //Reference count decreases to 0; memory allocated for window gets deleted here.
}
