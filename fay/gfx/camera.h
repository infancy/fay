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
        glm::vec3 position = glm::vec3(0.f, 0.f, 0.f),
        float yaw = -90.f, float pitch = 0.f, float _near = 1.f, float _far = 1000.f)
        : position_{ position }
        , yaw_{ yaw }
        , pitch_{ pitch }
        , near_ { _near }
        , far_{ _far }
	{
        // calculate 'front', 'up_' and 'right_'
		update_camera();
	}

    glm::vec3 position() const { return position_; }

    float zoom() const { return zoom_; }

    // view_matrix
	glm::mat4 view() const
	{
		return glm::lookAt(position_, position_ + front_, up_);
	}

    glm::mat4 persp() const
    {
        return glm::perspective(
            glm::radians(zoom()), aspect_, near_, far_);
    }

    glm::mat4 ortho() const
    {
        return glm::ortho(-50.f, 50.f, -50.f, 50.f, 1.f, 100.f);
    }

    // actually transform to homogeneous clipping space
    glm::mat4 world_to_ndc(bool use_perspective = true) const
    {
        return (use_perspective ? persp() : ortho()) * view();
    }

    // TODO
    // bool on_window_resize_event(const window_resize_event& event)

    bool on_input_event(const fay::single_input& io)
    {
        // if(active)
        ProcessMouseMovement(io.dx, io.dy);
        ProcessKeyboard(io);
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
    void ProcessKeyboard(const fay::single_input& io)
    {
        float velocity = mouse_movement_speed_ * io.delta_time;

        if (io['w']) position_ += front_ * velocity;
        if (io['s']) position_ -= front_ * velocity;
        if (io['a']) position_ -= right_ * velocity;
        if (io['d']) position_ += right_ * velocity;
        if (io.left_down)  position_ += up_ * velocity;
        if (io.right_down) position_ -= up_ * velocity;
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
		right_ = glm::normalize(glm::cross(front_, WorldUp_));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		up_ = glm::normalize(glm::cross(right_, front_));
	}

private:
    static inline float Zoom_{ 90.f };
    static inline glm::vec3 WorldUp_{ glm::vec3(0.f, 1.f, 0.f) };

    glm::vec3 position_;
    glm::vec3 front_; // glm::vec3(0.f, 0.f, -1.f)
    glm::vec3 up_;    // glm::vec3(0.f, 1.f, 0.f)
    glm::vec3 right_;
    // glm::mat4 view_matrix_{ 1.f };

    // Eular Angles
    float yaw_; // x
    float pitch_; // y

    // camera options
    float mouse_movement_speed_{ 10.f };
    float mouse_sensitivity_{ 0.2f };

    float zoom_{ Zoom_ };
    float aspect_{ 1080.f / 720.f };
    float near_;
    float far_;
};

} // namespace fay
