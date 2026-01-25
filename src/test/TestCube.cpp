// src/test/TestCube.cpp
#include "TestCube.h"
#include "log/Logger.h"
#include "render/ShaderController.h"
#include "input/InputController.h"
#include "managers/CameraManager.h"
#include "render/Camera.h"

#include "render/material/Material.h"      // <-- ЭТО ВАЖНО!
#include "render/texture/Texture.h"        // <-- И ЭТО!

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

namespace ogle {

	TestCube::TestCube() = default;

	TestCube::~TestCube() {
		Shutdown();
	}

	bool TestCube::Initialize() {
		Logger::Info("TestCube initializing...");

		// Сначала проверим все builtin шейдеры
		TestBuiltinShaders();

		auto& shaderCtrl = ShaderController::Get();

		m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);

		if (!m_shaderProgram) {
			Logger::Error("Failed to create shader");
			return false;
		}

		CreateGeometry();

		// Настройка системы ввода!
		SetupInputActions();

		// СОЗДАЕМ МАТЕРИАЛЫ И ТЕКСТУРЫ
		SetupMaterials();

		Logger::Info("TestCube initialized");
		return true;
	}

	void TestCube::SetupMaterials() {
		auto& materialCtrl = MaterialController::Get();
		auto& textureCtrl = TextureController::Get();

		Logger::Info("=== Setting up Materials ===");

		// 1. БАЗОВЫЙ ЦВЕТНОЙ МАТЕРИАЛ
		auto* colorMaterial = materialCtrl.CreateMaterial("Cube_Colorful", MaterialType::Basic);
		if (colorMaterial) {
			colorMaterial->SetShader(m_shaderProgram.get());
			colorMaterial->SetVec4("uColor", glm::vec4(0.2f, 0.8f, 0.2f, 1.0f)); // Зеленый
			colorMaterial->SetFloat("uUseLighting", 1.0f);
			m_materialNames.push_back("Colorful");
			Logger::Info("Material created: Colorful");
		}

		// 2. КРАСНЫЙ МАТЕРИАЛ (инстанс)
		auto* redMaterial = materialCtrl.CreateInstance("Cube_Red", "Cube_Colorful");
		if (redMaterial) {
			redMaterial->SetVec4("uColor", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f)); // Красный
			m_materialNames.push_back("Red");
			Logger::Info("Material created: Red (instance)");
		}

		// 3. СИНИЙ МАТЕРИАЛ (инстанс)
		auto* blueMaterial = materialCtrl.CreateInstance("Cube_Blue", "Cube_Colorful");
		if (blueMaterial) {
			blueMaterial->SetVec4("uColor", glm::vec4(0.2f, 0.2f, 0.8f, 1.0f)); // Синий
			m_materialNames.push_back("Blue");
			Logger::Info("Material created: Blue (instance)");
		}

		// 4. МАТЕРИАЛ С ТЕКСТУРОЙ (шахматная доска)
		// Сначала создаем текстуру
		auto* checkerTexture = textureCtrl.CreateTexture2D("Checkerboard", 2, 2);
		if (checkerTexture) {
			// Данные для шахматной доски 2x2
			unsigned char data[] = {
				255, 255, 255, 255,   // белый
				0, 0, 0, 255,         // черный
				0, 0, 0, 255,         // черный
				255, 255, 255, 255    // белый
			};

			// Приводим к Texture2D и загружаем
			if (auto* tex2D = dynamic_cast<Texture2D*>(checkerTexture)) {
				tex2D->LoadFromMemory(data, 2, 2, 4);
			}

			// Создаем материал с текстурой
			auto* texturedMaterial = materialCtrl.CreateMaterial("Cube_Textured", MaterialType::Basic);
			if (texturedMaterial) {
				texturedMaterial->SetShader(m_shaderProgram.get());
				texturedMaterial->SetVec4("uColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Белый
				texturedMaterial->SetFloat("uUseLighting", 1.0f);
				// TODO: Привязать текстуру когда будет система текстур в материалах
				m_materialNames.push_back("Textured");
				Logger::Info("Material created: Textured");
			}
		}

		// 5. МАТЕРИАЛ ДЛЯ ПРОВОЛОЧНОЙ МОДЕЛИ
		auto* wireframeMaterial = materialCtrl.CreateMaterial("Cube_Wireframe", MaterialType::Basic);
		if (wireframeMaterial) {
			wireframeMaterial->SetShader(m_shaderProgram.get());
			wireframeMaterial->SetVec4("uColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Белый
			wireframeMaterial->SetFloat("uUseLighting", 0.0f);
			wireframeMaterial->GetRenderState().wireframe = true;
			wireframeMaterial->GetRenderState().lineWidth = 2.0f;
			m_materialNames.push_back("Wireframe");
			Logger::Info("Material created: Wireframe");
		}

		// Устанавливаем начальный материал
		if (!m_materialNames.empty()) {
			m_currentMaterial = materialCtrl.GetMaterial("Cube_Colorful");
			m_materialIndex = 0;
			Logger::Info("Current material set to: " + m_materialNames[m_materialIndex]);
		}

		// ДЕМОНСТРАЦИЯ: создаем еще несколько материалов через встроенные
		auto* whiteMaterial = materialCtrl.GetBuiltin(MaterialController::Builtin::DefaultWhite);
		auto* redBuiltin = materialCtrl.GetBuiltin(MaterialController::Builtin::DefaultRed);
		auto* blueBuiltin = materialCtrl.GetBuiltin(MaterialController::Builtin::DefaultBlue);

		if (whiteMaterial && redBuiltin && blueBuiltin) {
			Logger::Info("Builtin materials loaded successfully");
		}

		Logger::Info("=== Materials setup complete ===");
	}


	bool TestCube::TestBuiltinShaders() {
		Logger::Info("=== Testing Builtin Shaders ===");

		auto& shaderCtrl = ShaderController::Get();

		// Тестируем каждый builtin шейдер
		bool allPassed = true;

		// 1. BasicColor
		auto basicColor = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);
		if (!basicColor) {
			Logger::Error("BasicColor shader failed");
			allPassed = false;
		}
		else {
			Logger::Info("✓ BasicColor: OK (ID: " + std::to_string(basicColor->GetID()) + ")");
		}

		// 2. BasicTexture  
		auto basicTexture = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicTexture);
		if (!basicTexture) {
			Logger::Error("BasicTexture shader failed");
			allPassed = false;
		}
		else {
			Logger::Info("✓ BasicTexture: OK (ID: " + std::to_string(basicTexture->GetID()) + ")");
		}

		// 3. Skybox
		auto skybox = shaderCtrl.GetBuiltin(ShaderController::Builtin::Skybox);
		if (!skybox) {
			Logger::Error("Skybox shader failed");
			allPassed = false;
		}
		else {
			Logger::Info("✓ Skybox: OK (ID: " + std::to_string(skybox->GetID()) + ")");
		}

		// 4. Unlit
		auto unlit = shaderCtrl.GetBuiltin(ShaderController::Builtin::Unlit);
		if (!unlit) {
			Logger::Error("Unlit shader failed");
			allPassed = false;
		}
		else {
			Logger::Info("✓ Unlit: OK (ID: " + std::to_string(unlit->GetID()) + ")");
		}

		// 5. Wireframe
		auto wireframe = shaderCtrl.GetBuiltin(ShaderController::Builtin::Wireframe);
		if (!wireframe) {
			Logger::Error("Wireframe shader failed");
			allPassed = false;
		}
		else {
			Logger::Info("✓ Wireframe: OK (ID: " + std::to_string(wireframe->GetID()) + ")");
		}

		if (allPassed) {
			Logger::Info("=== All builtin shaders PASSED ===");
		}
		else {
			Logger::Error("=== Some builtin shaders FAILED ===");
		}

		return allPassed;
	}

	void TestCube::SetShaderMode(ShaderMode mode) {
		auto& shaderCtrl = ShaderController::Get();

		switch (mode) {
		case ShaderMode::Colorful:
			// Используем существующий шейдер
			Logger::Info("Switched to Colorful shader");
			break;

		case ShaderMode::BasicColor:
			m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);
			Logger::Info("Switched to BasicColor shader");
			break;

		case ShaderMode::BasicTexture:
			m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicTexture);
			Logger::Info("Switched to BasicTexture shader");
			break;

		case ShaderMode::Unlit:
			m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::Unlit);
			Logger::Info("Switched to Unlit shader");
			break;

		case ShaderMode::Wireframe:
			m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::Wireframe);
			Logger::Info("Switched to Wireframe shader");
			break;
		}
	}

	void TestCube::Shutdown() {
		if (m_vao != 0) {
			glDeleteVertexArrays(1, &m_vao);
			m_vao = 0;
		}
		if (m_vbo != 0) {
			glDeleteBuffers(1, &m_vbo);
			m_vbo = 0;
		}
		if (m_ebo != 0) {
			glDeleteBuffers(1, &m_ebo);
			m_ebo = 0;
		}

		m_shaderProgram.reset();
		Logger::Debug("TestCube shutdown");
	}

	void TestCube::CreateGeometry() {
		float vertices[] = {
			-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,

			-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,

			-0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,

			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,

			-0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f
		};

		unsigned int indices[] = {
			0,1,2,2,3,0, 4,5,6,6,7,4,
			8,9,10,10,11,8, 12,13,14,14,15,12,
			16,17,18,18,19,16, 20,21,22,22,23,20
		};

		glGenVertexArrays(1, &m_vao);
		glGenBuffers(1, &m_vbo);
		glGenBuffers(1, &m_ebo);

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void TestCube::Render(float time, const glm::mat4& view, const glm::mat4& projection) {
		if (!m_shaderProgram || m_vao == 0) return;
		// ЕСЛИ ЕСТЬ МАТЕРИАЛ - ИСПОЛЬЗУЕМ ЕГО
		if (m_currentMaterial && m_currentMaterial->GetShader()) {
			// Используем шейдер из материала
			m_currentMaterial->GetShader()->Bind();
			// Применяем материал
			m_currentMaterial->Apply(m_currentMaterial->GetShader());
			// Применяем состояние рендеринга из материала
			m_currentMaterial->GetRenderState().Apply();
			// Используем шейдер из материала для матриц
			auto* materialShader = m_currentMaterial->GetShader();
			materialShader->SetMat4("uModel",
				glm::rotate(glm::mat4(1.0f), time * m_testRotationSpeed, glm::vec3(0.5f, 1.0f, 0.0f)));
			materialShader->SetMat4("uView", view);
			materialShader->SetMat4("uProjection", projection);
			// Настройки для разных типов материалов
			std::string materialName = m_currentMaterial->GetName();
			if (materialName.find("Wireframe") != std::string::npos) {
				// Для проволочного режима
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glLineWidth(2.0f);
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}
		else {
			// СТАРЫЙ КОД (если нет материала)
			m_shaderProgram->Bind();
			// Модельная матрица (вращение куба)
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::rotate(model, time * m_testRotationSpeed, glm::vec3(0.5f, 1.0f, 0.0f));
			// Используем переданные матрицы камеры
			m_shaderProgram->SetMat4("uModel", model);
			m_shaderProgram->SetMat4("uView", view);
			m_shaderProgram->SetMat4("uProjection", projection);
			// Настройки для разных типов шейдеров
			if (m_shaderProgram->GetName().find("BasicColor") != std::string::npos) {
				m_shaderProgram->SetVec3("uColor", glm::vec3(0.2f, 0.8f, 0.2f)); // Зеленый
				m_shaderProgram->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
				m_shaderProgram->SetInt("uUseLighting", 1);
			}
			else if (m_shaderProgram->GetName().find("Unlit") != std::string::npos) {
				m_shaderProgram->SetVec3("uColor", glm::vec3(0.8f, 0.2f, 0.8f)); // Пурпурный
				m_shaderProgram->SetInt("uUseTexture", 0); // Без текстуры
			}
			else if (m_shaderProgram->GetName().find("Wireframe") != std::string::npos) {
				m_shaderProgram->SetVec3("uColor", glm::vec3(1.0f, 1.0f, 1.0f)); // Белый
				m_shaderProgram->SetInt("uShowNormals", 0); // Не показывать нормали
			}
			else if (m_shaderProgram->GetName().find("BasicTexture") != std::string::npos) {
				m_shaderProgram->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
				m_shaderProgram->SetInt("uUseLighting", 1);
			}
		}
		// Рисуем куб
		glBindVertexArray(m_vao);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		// Восстанавливаем режим полигонов
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// Отключаем шейдер
		if (m_currentMaterial && m_currentMaterial->GetShader()) {
			m_currentMaterial->GetShader()->Unbind();
		}
		else {
			m_shaderProgram->Unbind();
		}
	}

	void TestCube::SetupInputActions() {
		auto& input = InputController::Get();

		Logger::Info("=== Setting up Input Actions ===");

		// 1. ДЕЙСТВИЕ: Переключение шейдеров
		auto* switchAction = input.CreateAction("SwitchShader", ActionType::Button);
		switchAction->AddKey('F');  // Клавиша F
		switchAction->AddKey(VK_TAB);  // Или Tab

		switchAction->OnPressed([this](const ActionState& state) {
			Logger::Info("SwitchShader action pressed!");
			CycleShader();
			});

		// 2. ДЕЙСТВИЕ: Включение/выключение управления камерой
		auto* cameraToggleAction = input.CreateAction("ToggleCameraControl", ActionType::Button);
		cameraToggleAction->AddKey(VK_SPACE);  // Пробел

		cameraToggleAction->OnPressed([this](const ActionState& state) {
			m_cameraControlEnabled = !m_cameraControlEnabled;
			Logger::Info(m_cameraControlEnabled ?
				"Camera control ENABLED" : "Camera control DISABLED");
			});

		// 3. ДЕЙСТВИЕ: Сброс камеры
		auto* resetCameraAction = input.CreateAction("ResetCamera", ActionType::Button);
		resetCameraAction->AddKey('R');

		resetCameraAction->OnPressed([](const ActionState& state) {
			Logger::Info("ResetCamera action pressed!");
			auto& cameraMgr = CameraManager::Get();
			Camera* camera = cameraMgr.GetMainCamera();
			if (camera) {
				camera->SetPosition({ 2.0f, 2.0f, 3.0f });
				camera->LookAt({ 0.0f, 0.0f, 0.0f });
				camera->SetMode(Camera::Mode::Free);
			}
			});

		// 4. ДЕЙСТВИЕ: Изменение скорости вращения (ось)
		auto* rotationSpeedAction = input.CreateAction("RotationSpeed", ActionType::Axis);
		rotationSpeedAction->AddAxisPair('N', 'M');  // N - медленнее, M - быстрее

		rotationSpeedAction->OnHeld([this](const ActionState& state) {
			float speedDelta = state.value * 0.1f; // Меньше изменения
			m_testRotationSpeed += speedDelta;
			// Ограничиваем диапазон
			if (m_testRotationSpeed < 0.1f) m_testRotationSpeed = 0.1f;
			if (m_testRotationSpeed > 5.0f) m_testRotationSpeed = 5.0f;
			});

		// 5. ДЕЙСТВИЕ: Вращение куба стрелками
		auto* rotateCubeAction = input.CreateAction("RotateCube", ActionType::Vector2);
		// Используем стрелки для вращения
		rotateCubeAction->AddKey(VK_LEFT, Modifiers{}, -1.0f);  // Влево
		rotateCubeAction->AddKey(VK_RIGHT, Modifiers{}, 1.0f);  // Вправо
		rotateCubeAction->AddKey(VK_UP, Modifiers{}, 1.0f);     // Вверх
		rotateCubeAction->AddKey(VK_DOWN, Modifiers{}, -1.0f);  // Вниз


		// 6. ДЕЙСТВИЕ: Переключение материалов
		auto* materialAction = input.CreateAction("SwitchMaterial", ActionType::Button);
		materialAction->AddKey('Z');  // Клавиша M

		materialAction->OnPressed([this](const ActionState& state) {
			Logger::Info("SwitchMaterial action pressed!");
			SwitchToNextMaterial();
			});

		// 7. ДЕЙСТВИЕ: Переключение проволочного режима
		auto* wireframeAction = input.CreateAction("ToggleWireframe", ActionType::Button);
		wireframeAction->AddKey('X');  // Клавиша W

		wireframeAction->OnPressed([this](const ActionState& state) {
			Logger::Info("ToggleWireframe action pressed!");
			ToggleWireframe();
			});

		// 8. ДЕЙСТВИЕ: Создание нового материала
		auto* createMaterialAction = input.CreateAction("CreateMaterial", ActionType::Button);
		createMaterialAction->AddKey('C', Modifiers{ true });  // Ctrl+C

		createMaterialAction->OnPressed([this](const ActionState& state) {
			Logger::Info("CreateMaterial action pressed!");
			// TODO: Создать случайный материал
			auto& materialCtrl = MaterialController::Get();

			// Создаем случайный цвет
			glm::vec4 randomColor = {
				(rand() % 100) / 100.0f,
				(rand() % 100) / 100.0f,
				(rand() % 100) / 100.0f,
				1.0f
			};

			std::string name = "Random_" + std::to_string(rand() % 1000);
			auto* newMaterial = materialCtrl.CreateMaterial(name, MaterialType::Basic);
			if (newMaterial) {
				newMaterial->SetShader(m_shaderProgram.get());
				newMaterial->SetVec4("uColor", randomColor);
				Logger::Info("Random material created: " + name);
			}
			});

		Logger::Info("Input actions setup complete");
	}

	void TestCube::Update(float deltaTime) {
		//m_ui.Update(deltaTime);
		auto& input = InputController::Get();
		// === ПЕРЕКЛЮЧЕНИЕ МАТЕРИАЛОВ ===
		static bool mWasPressed = false;
		if (input.IsKeyPressed('M') && !mWasPressed) {
			SwitchToNextMaterial();
			mWasPressed = true;
		}
		else if (!input.IsKeyPressed('M')) {
			mWasPressed = false;
		}

		// === ПЕРЕКЛЮЧЕНИЕ ПРОВОЛОЧНОГО РЕЖИМА ===
		static bool wWasPressed = false;
		if (input.IsKeyPressed('W') && !wWasPressed) {
			ToggleWireframe();
			wWasPressed = true;
		}
		else if (!input.IsKeyPressed('W')) {
			wWasPressed = false;
		}
		// ПРЯМАЯ ПРОВЕРКА КЛАВИШ
		if (input.IsKeyPressed(VK_SPACE)) {
			Logger::Info("=== SPACE pressed directly! ===");
		}

		if (input.IsKeyPressed('R')) {
			Logger::Info("=== R pressed directly! ===");
		}

		if (input.IsKeyPressed('N')) {
			Logger::Info("=== N pressed directly! ===");
		}

		if (input.IsKeyPressed('M')) {
			Logger::Info("=== M pressed directly! ===");
		}
		// === ТЕСТИРОВАНИЕ ОСЕЙ ===

		// 1. Получаем значения стандартных осей
		float horizontal = input.GetAxis("Horizontal");
		float vertical = input.GetAxis("Vertical");
		float mouseX = input.GetAxis("MouseX");
		float mouseY = input.GetAxis("MouseY");

		// Логируем значения осей (для отладки)
		m_axisLogTimer += deltaTime;
		//if (m_axisLogTimer > 2.0f) {  // Каждые 2 секунды
		//    Logger::Debug("Axes - H: " + std::to_string(horizontal) + 
		//                 " V: " + std::to_string(vertical) +
		//                 " MX: " + std::to_string(mouseX) +
		//                 " MY: " + std::to_string(mouseY));
		//    m_axisLogTimer = 0.0f;
		//}

		// 2. Тестируем кастомные действия
		auto* rotationSpeedAction = input.GetAction("RotationSpeed");
		if (rotationSpeedAction) {
			float speedDelta = rotationSpeedAction->GetState().value * deltaTime * 0.5f;
			m_testRotationSpeed += speedDelta;
			// Ограничиваем
			if (m_testRotationSpeed < 0.1f) m_testRotationSpeed = 0.1f;
			if (m_testRotationSpeed > 5.0f) m_testRotationSpeed = 5.0f;
		}

		// 3. Управление камерой (если включено)
		if (m_cameraControlEnabled) {
			ProcessCameraInput(deltaTime);
		}

		// 4. Проверка клавиш напрямую (для сравнения)
		if (input.IsKeyPressed('T')) {
			Logger::Info("T key pressed directly!");
		}
	}

	void TestCube::ProcessCameraInput(float deltaTime) {
		auto& input = InputController::Get();
		auto& cameraMgr = CameraManager::Get();
		Camera* camera = cameraMgr.GetMainCamera();

		if (!camera) return;


		// ДОБАВЬ ОТЛАДКУ
		//Logger::Debug("Camera control: " + std::string(m_cameraControlEnabled ? "ENABLED" : "DISABLED"));
		//Logger::Debug("Right mouse: " + std::string(input.IsMouseButtonDown(1) ? "DOWN" : "UP"));
		//Logger::Debug("Mouse delta: " +
		//    std::to_string(input.GetMouseDelta().x) + ", " +
		//    std::to_string(input.GetMouseDelta().y));


		// 1. Движение через оси
		float moveHorizontal = input.GetAxis("Horizontal");
		float moveVertical = input.GetAxis("Vertical");

		// Q/E или Space/Ctrl для движения вверх/вниз
		float moveUp = 0.0f;
		if (input.IsKeyDown('Q') || input.IsKeyDown(VK_SPACE)) moveUp += 1.0f;
		if (input.IsKeyDown('E') || input.IsKeyDown(VK_CONTROL)) moveUp -= 1.0f;

		// Применяем движение
		float moveSpeed = 5.0f;
		camera->MoveRight(moveHorizontal * moveSpeed * deltaTime);
		camera->MoveForward(moveVertical * moveSpeed * deltaTime);
		camera->MoveUp(moveUp * moveSpeed * deltaTime);

		// 2. Вращение через мышь (при зажатой правой кнопке)
		bool rightMouseDown = input.IsMouseButtonDown(1);

		if (rightMouseDown && !m_rightMouseDown) {
			// Запоминаем позицию мыши
			m_lastMousePos = input.GetMousePosition();
		}
		m_rightMouseDown = rightMouseDown;

		if (rightMouseDown) {
			// Получаем дельту через оси мыши
			float mouseX = input.GetAxis("MouseX");
			float mouseY = input.GetAxis("MouseY");

			if (std::abs(mouseX) > 0.01f || std::abs(mouseY) > 0.01f) {
				camera->ProcessMouseMovement(mouseX * 0.5f, mouseY * 0.5f);
			}
		}

		// 3. Zoom через колесико
		float mouseWheel = input.GetMouseWheelDelta();
		if (std::abs(mouseWheel) > 0.01f) {
			camera->ProcessMouseScroll(mouseWheel);
		}
	}

	void TestCube::CycleShader() {
		auto& shaderCtrl = ShaderController::Get();

		static int currentShader = 0;
		currentShader = (currentShader + 1) % 4; // Только 4 шейдера для куба!

		switch (currentShader) {
		case 0:
			m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);
			Logger::Info("Switched to: BasicColor shader");
			break;
		case 1:
			m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicTexture);
			Logger::Info("Switched to: BasicTexture shader");
			break;
		case 2:
			m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::Unlit);
			Logger::Info("Switched to: Unlit shader");
			break;
		case 3:
			m_shaderProgram = shaderCtrl.GetBuiltin(ShaderController::Builtin::Wireframe);
			Logger::Info("Switched to: Wireframe shader");
			break;
			// Skybox убран - он не для кубов!
		}
	}

	void TestCube::SwitchToNextMaterial() {
		if (m_materialNames.empty()) return;

		auto& materialCtrl = MaterialController::Get();

		// Переключаем индекс
		m_materialIndex = (m_materialIndex + 1) % m_materialNames.size();

		// Получаем материал по имени
		std::string materialName;
		switch (m_materialIndex) {
		case 0: materialName = "Cube_Colorful"; break;
		case 1: materialName = "Cube_Red"; break;
		case 2: materialName = "Cube_Blue"; break;
		case 3: materialName = "Cube_Textured"; break;
		case 4: materialName = "Cube_Wireframe"; break;
		default: materialName = "Cube_Colorful";
		}

		m_currentMaterial = materialCtrl.GetMaterial(materialName);

		if (m_currentMaterial) {
			Logger::Info("Switched to material: " + m_materialNames[m_materialIndex]);
		}
	}

	void TestCube::ToggleWireframe() {
		m_wireframeMode = !m_wireframeMode;
		if (m_currentMaterial) {
			auto& renderState = m_currentMaterial->GetRenderState();
			renderState.wireframe = m_wireframeMode;
			renderState.lineWidth = m_wireframeMode ? 2.0f : 1.0f;
			Logger::Info("Wireframe mode: " + std::string(m_wireframeMode ? "ON" : "OFF"));
		}
	}


} // namespace ogle