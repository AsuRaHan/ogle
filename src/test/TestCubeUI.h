// src/test/TestCubeUI.h
#pragma once

namespace ogle {

class TestCube;

class TestCubeUI {
public:
    explicit TestCubeUI(TestCube* cube = nullptr);
    
    void Initialize();
    void Update(float deltaTime);
    
private:
    TestCube* m_cube = nullptr;
    float m_time = 0.0f;
    int m_button1Clicks = 0;
    int m_button2Clicks = 0;
};

} // namespace ogle