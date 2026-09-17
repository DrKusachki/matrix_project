#include "config.h"

void Shutdown();
void DrawStep();
IMatrix<float> * CreateSparseMatrix( int, int, int );
IMatrix<float> * CreateOrdinaryMatrix( int, int, int );
void GenerateOrdinaryMatrix(IMatrix<float>*&, int, int, int);
void GenerateSparseMatrix(IMatrix<float>*&, int, int, int);
void Switch( IMatrix<float> *& );
void Undecorate( IMatrix<float> *& );
void Draw(MatrixDrawingAPI<float> &, IMatrix<float>*, bool);
void Remove( IMatrix<float> *& );

int main()
{
    GLFWwindow * window;
    if ( !glfwInit() )
      return -1;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    window = glfwCreateWindow(800, 600, "Matrix drawing", NULL, NULL);
    glfwMakeContextCurrent(window);

    if ( !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        return -1;
    }

    glClearColor(.25f,.3f,0.4f,1.0f);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();


    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawStep();
        
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }


    Shutdown();
}

void Shutdown()
{
    glfwTerminate();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DrawStep()
{
  static IMatrix<float> * mainMatrix; // The matrix object, drawn in the window.
  static IMatrix<float> * group; // Matrix group. Made in a separate window, then attached to mainMatrix.
  static IMatrix<float> * toGroup; // Matrix to attach to group.

  static PureTextMatrix<float> textAPI;
  static ImGuiMatrix<float> drawAPI;
  static bool drawEdge = true, canDraw = false, groupConstructor = false;
  static int rows = 2, cols = 2, nonzeros = 2, drawMode = 0;
  
  ImGui::SetNextWindowSize( ImVec2( 800.0f, 600.0f ), ImGuiCond_Appearing );
  ImGui::SetNextWindowPos( ImVec2( 0.0f, 0.0f ), ImGuiCond_Appearing );
  ImGui::Begin( "Matrix thingamajig", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse );
  {
    if( ImGui::BeginTable("##01", 2, ImGuiTabBarFlags_FittingPolicyResizeDown ) )
    {
      if ( !canDraw && mainMatrix )
        canDraw = true;
      ImGui::TableNextColumn();
      ImGui::SliderInt( "##02", &rows, 1, 20, "%2d", ImGuiSliderFlags_ClampZeroRange | ImGuiSliderFlags_NoInput ); ImGui::TableNextColumn();
      ImGui::Text( "Rows in the matrix" ); ImGui::TableNextColumn();
      ImGui::SliderInt( "##03", &cols, 1, 20, "%2d", ImGuiSliderFlags_ClampZeroRange | ImGuiSliderFlags_NoInput ); ImGui::TableNextColumn();
      ImGui::Text( "Cols in the matrix" ); ImGui::TableNextColumn();
      if ( nonzeros > cols * rows )
        nonzeros = cols * rows;
      ImGui::SliderInt( "##04", &nonzeros, 1, cols * rows, "%2d", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput ); ImGui::TableNextColumn();
      ImGui::Text( "Non-zero elements in the matrix" ); ImGui::TableNextColumn();
      if ( ImGui::Button( "Generate ordinary matrix" ) )
        GenerateOrdinaryMatrix( mainMatrix, cols, rows, nonzeros );
      ImGui::TableNextColumn();
      if ( ImGui::Button( "Generate sparse matrix" ) )
        GenerateSparseMatrix( mainMatrix, cols, rows, nonzeros );
      ImGui::TableNextColumn();
      
      ImGui::BeginDisabled(!canDraw);
      {
        if ( ImGui::Button( "Draw as a text" ) )
          drawMode = 1;
        ImGui::TableNextColumn();
        if ( ImGui::Button( "Draw as a table" ) )
          drawMode = 2;
        ImGui::TableNextColumn();
        if ( ImGui::Button( "Mix matrix" ) )
          Switch( mainMatrix );
        ImGui::TableNextColumn();
        if ( ImGui::Button( "Unmix matrix" ) )
          Undecorate( mainMatrix );
        ImGui::TableNextColumn();
        if ( ImGui::Button( "Erase matrix" ) )
        {
          drawMode = 0;
          canDraw = false;
          Remove( mainMatrix );
        }
      } 
      ImGui::EndDisabled();
      
      ImGui::BeginDisabled( groupConstructor );
      {
        ImGui::TableNextColumn();
        if ( ImGui::Button( "Start drawing a group " ) )
          groupConstructor = true;
      }
      ImGui::EndDisabled();

      ImGui::TableNextColumn();
      ImGui::Checkbox( "Draw edges of the table", &drawEdge );
      ImGui::EndTable();
    }

    if ( drawMode == 1 )
      Draw( textAPI, mainMatrix, drawEdge );
    if ( drawMode == 2 )
      Draw(drawAPI, mainMatrix, drawEdge );
  }
  ImGui::End();
  ImGui::SetNextWindowSize( ImVec2( 400.0f, 300.0f ), ImGuiCond_Appearing );
  ImGui::SetNextWindowPos( ImVec2( ImGui::GetWindowSize().x / 2, ImGui::GetWindowSize().y / 2 ), ImGuiCond_Appearing );


  if ( groupConstructor )
  {
    static int groupCols = 1, groupRows = 1, groupNonzeroes = 1, groupDrawMode = 0;
    static bool groupCanDraw = false;
    ImGui::Begin( "Group constructor" );
    if ( ImGui::BeginTable( "##10", 2, ImGuiTabBarFlags_FittingPolicyResizeDown ) )
    {
      if ( !groupCanDraw && toGroup )
      {
        groupCanDraw = true;
        group = new HorizontalGrouping<float>;
      }
      ImGui::TableNextColumn();
      ImGui::SliderInt( "##11", &groupRows, 1, 20, "%2d", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput ); 
      ImGui::TableNextColumn();
      ImGui::Text( "Rows in the matrix" );
      ImGui::TableNextColumn();
      ImGui::SliderInt( "##12", &groupCols, 1, 20, "%2d", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput ); ImGui::TableNextColumn();
      ImGui::Text( "Cols in the matrix" ); ImGui::TableNextColumn();
      if ( groupNonzeroes > groupCols * groupRows )
        groupNonzeroes = groupCols * groupRows;
      ImGui::SliderInt( "##14", &groupNonzeroes, 1, groupCols * groupRows, "%2d", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput ); ImGui::TableNextColumn();
      ImGui::Text( "Non-zero elements in the matrix" ); ImGui::TableNextColumn();
      if ( ImGui::Button( "Generate ordinary matrix" ) )
      {
        toGroup = CreateOrdinaryMatrix( groupCols, groupRows, groupNonzeroes );
      }
      ImGui::TableNextColumn();
      if ( ImGui::Button( "Generate sparse matrix" ) )
      {
        toGroup = CreateSparseMatrix( groupCols, groupRows, groupNonzeroes );
      }
      ImGui::TableNextColumn();
      ImGui::BeginDisabled(!groupCanDraw);
      {
        if ( ImGui::Button( "Draw as a text" ) )
          groupDrawMode = 1;
        ImGui::TableNextColumn();
        if ( ImGui::Button( "Draw as a table" ) )
          groupDrawMode = 2;
        ImGui::TableNextColumn();
        if ( ImGui::Button( "Add to group" ) )
          static_cast<HorizontalGrouping<float> *>( group )->add( toGroup );
        ImGui::TableNextColumn();
        ImGui::BeginDisabled(group == nullptr || group->size() == std::pair<size_t, size_t>(0, 0));
        if ( ImGui::Button( "Add this to the main matrix" ) )
        {
          if ( !mainMatrix )
          {
            mainMatrix = new HorizontalGrouping<float>(group);
          }
          else
          {
            mainMatrix = new TransposingDecorator( mainMatrix );
            mainMatrix = new HorizontalGrouping <float>( mainMatrix );
            static_cast<HorizontalGrouping<float> *>( mainMatrix )->add( new TransposingDecorator(group) );
            mainMatrix = new TransposingDecorator( mainMatrix );
            // bit of a hack
            mainMatrix = new HorizontalGrouping <float>( mainMatrix );
          }
          group = nullptr;
          toGroup = nullptr;
          groupDrawMode = 0;
          groupConstructor = false;
          groupCanDraw = false;
        }
        ImGui::EndDisabled();
      } 
      ImGui::EndDisabled();
      ImGui::EndTable();
    }
    if ( groupDrawMode == 1 )
      Draw( textAPI, toGroup, true );
    if ( groupDrawMode == 2 )
      Draw(drawAPI, toGroup, true );
    
    ImGui::Text( "Group preview: " );
    if ( groupDrawMode == 1 )
      Draw( textAPI, group, true );
    if ( groupDrawMode == 2 )
      Draw( drawAPI, group, true );

    ImGui::End();
  }
}

void Remove( IMatrix<float> *& matr )
{
  if ( matr ) matr->~IMatrix();
  matr = nullptr;
}

void Switch( IMatrix<float> *& matr )
{
  matr = new SwitchingDecorator<float>( *matr );
  std::random_device rd;
  std::mt19937 gen(rd());

  size_t max = std::max( matr->numCols(), matr->numRows() );
  std::uniform_int_distribution<size_t> dist( 0, max );

  static_cast<SwitchingDecorator<float> *>( matr )->switchRows( dist(gen) % matr->numRows(), dist(gen) % matr->numRows() );
  static_cast<SwitchingDecorator<float> *>( matr )->switchColumns( dist(gen) % matr->numCols(), dist(gen) % matr->numCols() );
}

void Undecorate( IMatrix<float> *& matr )
{
  if ( matr->type() != "Decorated" )
    return;
  matr = const_cast<IMatrix<float>*>( matr->getComponent() );
}

IMatrix<float> * CreateOrdinaryMatrix( int cols, int rows, int nonzeroes )
{
  IMatrix<float> * matrix = new OrdinaryMatrix<float>( cols, rows );
  MatrixInitializer<float>::initMatrix( static_cast<AMatrix<float>*>( matrix ), nonzeroes, 100 );
  return matrix;
}

IMatrix<float> * CreateSparseMatrix( int cols, int rows, int nonzeroes )
{
  IMatrix<float> * matrix = new SparseMatrix<float>( cols, rows );
  MatrixInitializer<float>::initMatrix( static_cast<AMatrix<float>*>( matrix ), nonzeroes, 100 );
  return matrix;
}

void GenerateOrdinaryMatrix(IMatrix<float>*& matr, int cols, int rows, int nonzeroes)
{
  if ( matr )
    matr->~IMatrix();
  matr = new OrdinaryMatrix<float>(cols, rows);
  MatrixInitializer<float>::initMatrix( static_cast<AMatrix<float>*>( matr ), nonzeroes, 100 );
  printf_s( "Called generate ordinary!\n" );
}

void GenerateSparseMatrix(IMatrix<float>*& matr, int cols, int rows, int nonzeroes)
{
  if ( matr )
    matr->~IMatrix();
  matr = new SparseMatrix<float>(cols, rows);
  MatrixInitializer<float>::initMatrix( static_cast<AMatrix<float>*>( matr ), nonzeroes, 100 );
}

void Draw(MatrixDrawingAPI<float>& API, IMatrix<float>* matr, bool drawEdge)
{
  DrawingMatrix<float> drawing( API, *matr, drawEdge );
  if ( matr->size() != std::pair<size_t, size_t> ( 0, 0 ) )
    drawing.Draw();
}
