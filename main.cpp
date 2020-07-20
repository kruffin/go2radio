#include <go2/input.h>

#include <stdlib.h>
#include <iostream>

go2_gamepad_state_t outGamepadState;

int main() {

	go2_input_t* input = go2_input_create();

	while(1) {
		go2_input_gamepad_read(input,&outGamepadState);
		if (outGamepadState.buttons.f1) {
			std::cout << "f1";
			go2_input_destroy(input);
			return 0;
		}
	}
}
