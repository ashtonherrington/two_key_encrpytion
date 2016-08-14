Two key encryption/decryption program.

Usage instructions:

1. To encode your file, you must generate a key with more characters present than are in your file, 
to do so type "keygen [desired_number_of_characters] > [key_file_name]"

2. Set the encoding and decoding programs to run as daemons with the following commands:
"otp_enc_d [encoding_portnumber] &" and "otp_dec_d [decoding_portnumber] &"

3. Encoding: type command "otp_enc [file_to_encode] [key_file_name] [encoding_portnumber] > [ciphertext_name]"

4. Decoding: type command "otp_dec [ciphertext_name] [key_file_name] [decoding_portnumber] > [decoded_file_name]"
