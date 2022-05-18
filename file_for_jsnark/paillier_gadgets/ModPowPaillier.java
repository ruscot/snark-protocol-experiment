package examples.gadgets.paillier;

import java.math.BigInteger;

import circuit.auxiliary.LongElement;
import circuit.operations.Gadget;
import circuit.structure.Wire;
import circuit.structure.WireArray;
import examples.gadgets.math.LongIntegerModGadget;

public class ModPowPaillier extends Gadget {
    // every wire represents a byte in the following three arrays
	private Wire[] a;
	private int b;
	private Wire[] result;
	
	LongElement paillierModulus;
	int paillierKeyBitLength;
	public ModPowPaillier(LongElement paillierModulus, Wire[] a, int b, int paillierKeyBitLength, String... desc) {
		super(desc);
		this.paillierModulus = paillierModulus;
		this.paillierKeyBitLength = paillierKeyBitLength;
		this.a = a;
        this.b = b;
		buildCircuit();
	}

	private void buildCircuit() {
		// 1. safest method:
		/*WireArray aAllBits = new WireArray(a).getBits(8);
		LongElement aLongElement = new LongElement(aAllBits);
        WireArray bAllBits = new WireArray(b).getBits(8);
		LongElement bLongElement = new LongElement(bAllBits);*/
		WireArray allBitsA = new WireArray(a).getBits(32);
	 	LongElement msgA = new LongElement(allBitsA);
		LongElement res = new LongElement(
			new BigInteger[] { BigInteger.ONE });
		//Naive approach
		for(int i = 0; i < b; i++){
			res = res.mul(msgA);
			res = new LongIntegerModGadget(res, paillierModulus, paillierKeyBitLength, true).getRemainder();
		}
		
		//res = new LongIntegerModGadget(res, paillierModulus, paillierKeyBitLength, true).getRemainder();
		result = res.getBits(-1).packBitsIntoWords(8);
	}
	
	@Override
	public Wire[] getOutputWires() {
		return result;
	}
}
