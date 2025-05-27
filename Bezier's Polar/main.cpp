#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>

namespace Constants {
    const int WINDOW_WIDTH = 1200;
    const int WINDOW_HEIGHT = 1000;
};

using namespace Constants;

// Structure for 2D points
struct Point {
    float x, y;
};

bool dragging = false;
int draggedPointIndex = -1;
bool showPolar = false;

std::vector<Point> controlPoints = {
        {0.0f, 0.0f}, {0.5f, 0.8f}, {1.0f, 0.0f} // Example points
};

std::vector<Point> t_points; // Declare t_points for slider visualization
float SLIDER_VALUE = 0.5f;   // Default slider value

// De Casteljau algorithm for intermediate points
std::vector<Point> deCasteljau(const std::vector<Point>& controlPoints, float t) {
    std::vector<Point> newPoints;

    for (size_t i = 0; i < controlPoints.size() - 1; ++i) {
        float x = (1 - t) * controlPoints[i].x + t * controlPoints[i + 1].x;
        float y = (1 - t) * controlPoints[i].y + t * controlPoints[i + 1].y;
        newPoints.push_back({ x, y });
    }

    return newPoints;
}

// Compute a point on a Bézier curve
Point bezierPoint(const std::vector<Point>& controlPoints, float t) {
    std::vector<Point> tempPoints = controlPoints;

    while (tempPoints.size() > 1) {
        tempPoints = deCasteljau(tempPoints, t);
    }

    return tempPoints[0];
}

// OpenGL function to render a point as a small circle
void renderPoint(float x, float y, float radius = 0.05f) {
    const int segments = 20;
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * 3.14159265359f * float(i) / float(segments);
        float dx = radius * cos(angle);
        float dy = radius * sin(angle);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}

// OpenGL function to render lines between control points
void renderControlPolygon(const std::vector<Point>& controlPoints) {
    glBegin(GL_LINES);
    for (size_t i = 0; i < controlPoints.size() - 1; ++i) {
        glVertex2f(controlPoints[i].x, controlPoints[i].y);
        glVertex2f(controlPoints[i + 1].x, controlPoints[i + 1].y);
    }
    glEnd();
}

// OpenGL rendering function for Bézier curve
void renderBezier(const std::vector<Point>& controlPoints, const float color[3]) {
    const int numSegments = 100; 
    glColor3fv(color);          
    glBegin(GL_LINE_STRIP);

    for (int i = 0; i <= numSegments; ++i) {
        float t = i / (float)numSegments;
        Point bezierPt = bezierPoint(controlPoints, t);
        glVertex2f(bezierPt.x, bezierPt.y); // Plot the point
    }

    glEnd();
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float x_ndc = (xpos / WINDOW_WIDTH) * 3.0f - 1.5f;
        float y_ndc = ((WINDOW_HEIGHT - ypos) / WINDOW_HEIGHT) * 3.0f - 1.5f;

        if (action == GLFW_PRESS) {
            for (size_t i = 0; i < controlPoints.size(); ++i) {
                float dx = controlPoints[i].x - x_ndc;
                float dy = controlPoints[i].y - y_ndc;
                if (dx * dx + dy * dy < 0.05f * 0.05f) {
                    dragging = true;
                    draggedPointIndex = i;
                    break;
                }
            }
        }
        else if (action == GLFW_RELEASE) {
            t_points = deCasteljau(controlPoints, SLIDER_VALUE);
            dragging = false;
            draggedPointIndex = -1;
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (dragging && draggedPointIndex != -1) {
        float x_ndc = (xpos / WINDOW_WIDTH) * 3.0f - 1.5f;
        float y_ndc = ((WINDOW_HEIGHT - ypos) / WINDOW_HEIGHT) * 3.0f - 1.5f;
        controlPoints[draggedPointIndex] = { x_ndc, y_ndc };
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Bezier Curve Visualization", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 16);
    glfwMakeContextCurrent(window);
    glewInit();
    glEnable(GL_MULTISAMPLE);
    
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui window with buttons
        ImGui::Begin("Control Panel");

        if (ImGui::Button("Preset 1")) {
            t_points = {};
            controlPoints = { {0.0f, 0.0f}, {0.5f, 0.8f}, {1.0f, 0.0f} };
        }

        if (ImGui::Button("Preset 2")) {
            t_points = {};
            controlPoints = { {0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f} };
        }

        if (ImGui::Button("Preset 3")) {
            t_points = {};
            controlPoints = { {0.2f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {0.8f, 0.0f}, {0.4f, 0.3f} };
        }

        if (ImGui::Button("Add Control Point")) {
            t_points = {};
            controlPoints.push_back({ 0.5f, 0.5f });
        }

        if (ImGui::Button("Remove Last Control Point") && controlPoints.size() > 1) {
            t_points = {};
            controlPoints.pop_back();
        }
        if (ImGui::Checkbox("Show Polar", &showPolar)) {}

        if (ImGui::SliderFloat("Values of t1", &SLIDER_VALUE, 0.0f, 1.0f)) {
            t_points = deCasteljau(controlPoints, SLIDER_VALUE);
        }

        ImGui::Text("Control Points:");
        for (size_t i = 0; i < controlPoints.size(); ++i) {
            std::stringstream ss;
            ss << i;
            ImGui::InputFloat2(("Point " + ss.str()).c_str(), &controlPoints[i].x);
        }

        ImGui::End();

        // Render OpenGL
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1.5, 1.5, -1.5, 1.5, -1.0, 1.0);

        // Render the original Bézier curve
        float blue[3] = { 0.0f, 0.0f, 1.0f };
        renderBezier(controlPoints, blue);

        // Render control polygon
        glColor3f(1.0f, 1.0f, 1.0f);
        renderControlPolygon(controlPoints);

        // Render control points
        glColor3f(1.0f, 1.0f, 1.0f);
        for (const auto& point : controlPoints) {
            renderPoint(point.x, point.y);
        }

        // Visualize t_points
        if (!t_points.empty() && dragging == false && showPolar == true) {
            glColor3f(0.0f, 1.0f, 0.0f);
            for (const auto& point : t_points) {
                renderPoint(point.x, point.y, 0.03f);
            }

            glColor3f(0.0f, 0.5f, 0.0f);
            glBegin(GL_LINE_STRIP);
            for (const auto& point : t_points) {
                glVertex2f(point.x, point.y);
            }
            glEnd();

            // Render a new Bézier curve through t_points
            float red[3] = { 1.0f, 0.0f, 0.0f };
            renderBezier(t_points, red);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}