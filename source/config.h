#pragma once
#include <iostream>
#include <format>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "mathobjects.h"


template<typename T>
class MatrixDrawingAPI
{
public:
  virtual ~MatrixDrawingAPI() {};
  virtual void DrawMatrix( IMatrix<T>&, bool) const = 0;
};


template<typename T>
class ImGuiMatrix : public MatrixDrawingAPI<T>
{
public:
  void DrawMatrix( IMatrix<T>& matr, bool drawEdge ) const override
  {
    bool drawZeroes = ( matr.getComponent()->type() == "Ordinary" || matr.getComponent()->type() == "Horizontal matrix group" );

    if ( ImGui::BeginTable( "MatrixImGui", (int)matr.numCols(), drawEdge ? ImGuiTableFlags_BordersOuter : 0 ) )
    {
      for ( int item = 0; item < matr.numRows(); ++item )
        for ( int jtem = 0; jtem < matr.numCols(); ++jtem )
        {
          ImGui::TableNextColumn();
          T num = matr.getAt( item, jtem );
          if ( drawZeroes || ::abs(num) > 1.e-6 )
            ImGui::Text( "%8.4f", num );
        }
      ImGui::EndTable();
    }
  }
};

template <typename T>
class PureTextMatrix : public MatrixDrawingAPI<T>
{
public:
  void DrawMatrix(IMatrix<T>& matr, bool drawEdge) const override
  {
    if ( !&matr )
      return;
    bool drawZeroes = (matr.getComponent()->type() == "Ordinary" || matr.getComponent()->type() == "Horizontal matrix group" );
    std::string result;
    if ( drawEdge )
    {
      result += "+";
      for ( size_t i = 0; i < (matr.numCols() * 9); ++i )
        result += "-";
      result += "-+";
      result += '\n';
    }
    for ( size_t i = 0; i < matr.numRows(); ++i )
    {
      if ( drawEdge ) result += "| ";
      for ( size_t j = 0; j < matr.numCols(); ++j )
      {
        if ( drawZeroes || ::abs( matr.getAt( i, j ) ) > 1.e-8 )
          result += std::format( "{:^8.4f}", matr.getAt( i, j ) ) + " ";
        else
          result += "         ";
      }
      if ( drawEdge ) result += "|";
      result += "\n";
    }
    if ( drawEdge )
    {
      result += "+";
      for ( size_t i = 0; i < (matr.numCols() * 9); ++i )
        result += "-";
      result += "-+";
      result += '\n';
    }
    ImGui::Text( result.c_str() );
  }
};

template<typename T>
class DrawnThing {
protected:
  const MatrixDrawingAPI<T>& api;
public:
  DrawnThing( const MatrixDrawingAPI<T> & API ) : api( API ) {};
  virtual ~DrawnThing() = default;

  virtual void Draw() const = 0;
};

template<typename T>
class DrawingMatrix : public DrawnThing<T> {
private:
  using DrawnThing<T>::api;
  IMatrix<T>& matrix;
  bool drawEdges;
public:
  DrawingMatrix( const MatrixDrawingAPI<T> & api, IMatrix<T> & matr, bool dE ) :
    DrawnThing<T>( api ), matrix( matr ), drawEdges( dE ) {};
  void Draw() const override
  {
    return api.DrawMatrix( matrix, drawEdges );
  }
};
