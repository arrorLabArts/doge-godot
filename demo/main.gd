extends Node

func _ready():
	print("=== Dogecoin Wallet GDExtension Test ===")

	# Create wallet instance
	var wallet = DogeWallet.new()

	# Test 1: Generate new keypair
	print("\n--- Test 1: Generate Keypair ---")
	var keypair = wallet.generate_keypair(true, true)
	print("Private Key (WIF): ", keypair.private_key)
	print("Public Key (hex): ", keypair.public_key)
	print("Address: ", keypair.address)

	# Test 2: Import from WIF
	print("\n--- Test 2: Import from WIF ---")
	var imported = wallet.import_from_wif(keypair.private_key)
	print("Imported Address: ", imported.address)
	print("Matches original: ", imported.address == keypair.address)

	# Test 3: Validate address
	print("\n--- Test 3: Validate Address ---")
	var is_valid = wallet.validate_address(keypair.address, true)
	print("Address is valid: ", is_valid)

	# Test 4: Sign message
	print("\n--- Test 4: Sign Message ---")
	var message = "Hello, Dogecoin! To the moon!"
	var signature = wallet.sign_message_wif(message, keypair.private_key)
	print("Message: ", message)
	print("Signature: ", signature)

	# Test 5: Verify message
	print("\n--- Test 5: Verify Message ---")
	var is_verified = wallet.verify_message(message, signature, keypair.address)
	print("Signature verified: ", is_verified)

	# Test 6: Verify with wrong message (should fail)
	print("\n--- Test 6: Verify with Wrong Message ---")
	var wrong_verify = wallet.verify_message("Wrong message", signature, keypair.address)
	print("Signature verified (should be false): ", wrong_verify)

	print("\n=== All tests completed! ===")
