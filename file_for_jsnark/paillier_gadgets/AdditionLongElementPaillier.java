package examples.gadgets.paillier;

import circuit.auxiliary.LongElement;
import circuit.operations.Gadget;
import circuit.structure.Wire;
import circuit.structure.WireArray;
import examples.gadgets.math.LongIntegerModGadget;

/***
 * Gadget which perform addition in paillier
 * a + b [paillierModulus]
 */
public class AdditionLongElementPaillier extends Gadget {
	//Contain the first integer 
	private Wire[] a;
	//Contain the second integer
    private Wire[] b;

	private Wire[] result;
	
	private LongElement paillierModulus;
	private int paillierKeyBitLength;

	public AdditionLongElementPaillier(LongElement paillierModulus, Wire[] a, Wire[] b, int paillierKeyBitLength, String... desc) {
		super(desc);
		this.paillierModulus = paillierModulus;
		this.paillierKeyBitLength = paillierKeyBitLength;
		this.a = a;
        this.b = b;
		buildCircuit();
	}

	private void buildCircuit() {
		WireArray allBitsA = new WireArray(this.a).getBits(32);
	 	LongElement msgA = new LongElement(allBitsA);
		WireArray allBitsB = new WireArray(this.b).getBits(32);
	 	LongElement msgB = new LongElement(allBitsB);
		msgA = msgA.add(msgB);
		msgA = new LongIntegerModGadget(msgA, this.paillierModulus, this.paillierKeyBitLength, true).getRemainder();
		this.result = msgA.getBits(-1).packBitsIntoWords(8);
	}
	
	@Override
	public Wire[] getOutputWires() {
		return this.result;
	}
    
}
