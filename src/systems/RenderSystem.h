// src/systems/RenderSystem.h
#pragma once

#include <glad/gl.h>
#include <memory>

#include "core/ISystem.h"
#include "log/Logger.h"
#include "window/OpenGLContext.h"
#include "test/TestCube.h"

namespace ogle {

	class RenderSystem final : public ISystem {
	public:
		explicit RenderSystem(HDC hdc);
		~RenderSystem() override;

		const std::string& GetName() const override {
			static std::string n = "RenderSystem";
			return n;
		}

		bool Initialize() override;
		void Update(float deltaTime) override {}
		void Render() override;
		void Shutdown() override;

		void OnResize(int width, int height) override;
		void SetClearColor(float r, float g, float b, float a = 1.0f);

	private:
		HDC m_hdc;
		std::unique_ptr<OpenGLContext> m_context;
		float m_clearColor[4] = { 0.1f, 0.1f, 0.3f, 1.0f };

		///////////////////
		TestCube m_testCube;
		float m_time = 0.0f;
	};

} // namespace ogle