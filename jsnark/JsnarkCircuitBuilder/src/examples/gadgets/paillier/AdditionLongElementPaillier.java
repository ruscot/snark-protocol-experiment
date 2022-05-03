package examples.gadgets.paillier;

import circuit.auxiliary.LongElement;
import circuit.operations.Gadget;
import circuit.structure.Wire;
import circuit.structure.WireArray;
import examples.gadgets.math.LongIntegerModGadget;

public class AdditionLongElementPaillier extends Gadget {
    // every wire represents a byte in the following three arrays
	private Wire[] a;
    private Wire[] b;

	private Wire[] result;
	
	LongElement paillierModulus;
	int paillierKeyBitLength;
	public AdditionLongElementPaillier(LongElement paillierModulus, Wire[] a, Wire[] b, int paillierKeyBitLength, String... desc) {
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
		WireArray allBitsA = new WireArray(a).getBits(8);
	 	LongElement msgA = new LongElement(allBitsA);
		WireArray allBitsB = new WireArray(b).getBits(8);
	 	LongElement msgB = new LongElement(allBitsB);
		msgA = msgA.add(msgB);
		msgA = new LongIntegerModGadget(msgA, paillierModulus, paillierKeyBitLength, true).getRemainder();
		result = msgA.getBits(-1).packBitsIntoWords(8);
	}
	
	@Override
	public Wire[] getOutputWires() {
		return result;
	}
    
}
