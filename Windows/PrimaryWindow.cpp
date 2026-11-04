//
// Created by danie on 9/6/2026.
//

#include "PrimaryWindow.h"

PrimaryWindow::PrimaryWindow(UINT width, UINT height, std::wstring name) :
    BaseWindow(width, height, name),
    m_frameIndex(0),
    m_rtvDescriptorSize(0) {}

void PrimaryWindow::OnInit() {
    //LoadPipeline();
    //LoadAssets();
}

void PrimaryWindow::OnUpdate() {
}

void PrimaryWindow::OnRender() {
}

void PrimaryWindow::OnDestroy() {
}

void PrimaryWindow::LoadPipeline() {
}

void PrimaryWindow::LoadAssets() {
}

void PrimaryWindow::PopulateCommandList() {
}

void PrimaryWindow::WaitForPreviousFrame() {
}
