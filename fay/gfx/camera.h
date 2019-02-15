#pragma once

#include "fay/core/fay.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fay
{

// TODO
// eunum {ui, role, terrain, scene};

class camera
{
public:
	camera(
		glm::vec3 position = glm::vec3(0.f, 0.f, 0.f), glm::vec3 up = glm::vec3(0.f, 1.f, 0.f),
		float yaw = -90.f, 
        float pitch = 0.f)
        : position_{ position }
        , front_{ glm::vec3(0.0f, 0.0f, -1.0f) }
        , up_{ up }
        /*, right{}*/
        , world_up_{ up }
        , yaw_{ yaw }
        , pitch_{ pitch}
	{
        // calculate 'right_'
		update_camera();
	}

    glm::vec3 position() const { return position_; }

    float zoom() const { return zoom_; }

	glm::mat4 view_matrix() const
	{
		return glm::lookAt(position_, position_ + front_, up_);
	}

    bool on_input_event(const fay::single_input& io)
    {
        // if(active)
        ProcessMouseMovement(io.dx, io.dy);
        ProcessKeyboard({ io.key['w'], io.key['s'], io.key['a'], io.key['d'] }, io.delta_time);
        ProcessMouseScroll(io.wheel);

        // up_date front_, right_ and up_ Vectors using the updated Eular angles
        update_camera();

        return true;
    }

private:
    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainpitch_ = true)
    {
        xoffset *= mouse_sensitivity_;
        yoffset *= mouse_sensitivity_;

        yaw_ += xoffset;
        pitch_ += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainpitch_)
        {
            pitch_ = std::clamp(pitch_, -89.f, 89.f);
        }
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    // FORWARD, BACKWARD, LEFT, RIGHT
    void ProcessKeyboard(std::array<char, 4> direction, float deltaTime)
    {
        float velocity = mouse_movement_speed_ * deltaTime;
        if (direction[0]) position_ += front_ * velocity;
        if (direction[1]) position_ -= front_ * velocity;
        if (direction[2]) position_ -= right_ * velocity;
        if (direction[3]) position_ += right_ * velocity;
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        if (zoom_ >= 1.f && zoom_ <= Zoom_)
            zoom_ -= yoffset;

        zoom_ = std::clamp(zoom_, 1.f, Zoom_);
    }

	// Calculates the front vector from the camera's (updated) Eular Angles
	void update_camera()
	{
		// Calculate the new front_ vector
		glm::vec3 front;
		front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
		front.y = sin(glm::radians(pitch_));
		front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
		front_ = glm::normalize(front);

		// Also re-calculate the right_ and up_ vector
		right_ = glm::normalize(glm::cross(front_, world_up_));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		up_ = glm::normalize(glm::cross(right_, front_));
	}

private:
    static inline float Zoom_{ 90.f };

    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 world_up_;
    // Eular Angles
    float yaw_; // x
    float pitch_; // y
    // camera options
    float mouse_movement_speed_{ 10.f };
    float mouse_sensitivity_{ 0.2f };
    float zoom_{ Zoom_ };
};

} // namespace fay
