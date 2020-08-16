void gameUpdateAndRender(Game_framebuffer *framebuffer)
{  
  glViewport(0,0,framebuffer->width,framebuffer->height);
  glClearColor(1.0f,1.0f,0.0f,0.0f);
  glClear(GL_COLOR_BUFFER_BIT);  

  glBegin(GL_TRIANGLES);
  glColor3f(1.0f,0.0f,0.0f);
  glVertex2f(1.0f,0.0f);
  glVertex2f(0.0f,1.0f);
  glVertex2f(0.0f,0.0f);
  
  glEnd();
}
