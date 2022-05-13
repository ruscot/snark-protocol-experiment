package examples.gadgets.rsa;

import java.math.BigInteger;
import util.Util;
import circuit.auxiliary.LongElement;
import circuit.operations.Gadget;
import circuit.structure.Wire;
import circuit.structure.WireArray;
import examples.gadgets.math.DotProductGadget;
import examples.gadgets.math.FieldDivisionGadget;
import examples.gadgets.math.LongIntegerModGadget;
import examples.gadgets.math.ModConstantGadget;
import examples.gadgets.math.ModGadget;

public class PaillierEncryptionAddition_Gadget extends Gadget {
    private LongElement modulus;

	// every wire represents a byte in the following three arrays
	private Wire[] plainText;
	//private Wire[] randomness; // (rsaKeyBitLength / 8 - 3 - plainTextLength)
								// non-zero bytes
	private Wire[] ciphertext;

	private int rsaKeyBitLength; // in bits (assumed to be divisible by 8)

	public PaillierEncryptionAddition_Gadget(LongElement modulus, Wire[] plainText,
			/*Wire[] randomness,*/ int rsaKeyBitLength, String... desc) {
		super(desc);

		/*if (rsaKeyBitLength % 8 != 0) {
			throw new IllegalArgumentException(
					"RSA Key bit length is assumed to be a multiple of 8");
		}*/

		/*if (plainText.length > rsaKeyBitLength / 8 - 11
				|| plainText.length + randomness.length != rsaKeyBitLength / 8 - 3) {
			System.err.println("Check Message & Padding length");
			throw new IllegalArgumentException(
					"Invalid Argument Dimensions for RSA Encryption");
		}*/

		//this.randomness = randomness;
		this.plainText = plainText;
		this.modulus = modulus;
		this.rsaKeyBitLength = rsaKeyBitLength;
		buildCircuit();
	}

	public static int getExpectedRandomnessLength(int rsaKeyBitLength,
			int plainTextLength) {
		if (rsaKeyBitLength % 8 != 0) {
			throw new IllegalArgumentException(
					"RSA Key bit length is assumed to be a multiple of 8");

		}
		return rsaKeyBitLength / 8 - 3 - plainTextLength;
	}

	private void buildCircuit() {

		int lengthInBytes = rsaKeyBitLength;
		Wire[] paddedPlainText = new Wire[lengthInBytes];
		for (int i = 0; i < plainText.length; i++) {
			paddedPlainText[plainText.length - i - 1] = plainText[i];
		}
		paddedPlainText[plainText.length] = generator.getZeroWire();
		/*for (int i = 0; i < randomness.length; i++) {//How randomness length is calculated
			paddedPlainText[plainText.length + 1 + (randomness.length - 1) - i] = randomness[i];
		}
		paddedPlainText[lengthInBytes - 2] = generator.createConstantWire(2);
		paddedPlainText[lengthInBytes - 1] = generator.getZeroWire();*/

		/*
		 * To proceed with the RSA operations, we need to convert the
		 * padddedPlainText array to a long element. Two ways to do that.
		 */
		// 1. safest method:
		WireArray allBits = new WireArray(paddedPlainText).getBits(8);
		LongElement paddedMsg = new LongElement(allBits);


		// 2. Make multiple long integer constant multiplications (need to be
		// done carefully)
		/*LongElement paddedMsg = new LongElement(
				new BigInteger[] { BigInteger.ZERO });
		for (int i = 0; i < paddedPlainText.length; i++) {
			LongElement e = new LongElement(paddedPlainText[i], 8);
			LongElement c = new LongElement(Util.split(
					BigInteger.ONE.shiftLeft(8 * i),
					LongElement.CHUNK_BITWIDTH));
			paddedMsg = paddedMsg.add(e.mul(c));
		}*/
		
		LongElement s = paddedMsg;
		for (int i = 0; i < 16; i++) {
			s = s.mul(s);
			s = new LongIntegerModGadget(s, modulus, rsaKeyBitLength, false).getRemainder();
		}
		s = s.mul(paddedMsg);
		s = new LongIntegerModGadget(s, modulus, rsaKeyBitLength, true).getRemainder();

		// return the cipher text as byte array
		ciphertext = s.getBits(rsaKeyBitLength).packBitsIntoWords(8);
	}

	
	/*public void checkRandomnessCompliance(){
		// assert the randomness vector has non-zero bytes
		for (int i = 0; i < randomness.length; i++) {
			randomness[i].restrictBitLength(8);
			// verify that each element has a multiplicative inverse
			new FieldDivisionGadget(generator.getOneWire(), randomness[i]);
		}
	}*/
	
	@Override
	public Wire[] getOutputWires() {
		return ciphertext;
	}
}
