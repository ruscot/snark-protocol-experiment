package examples.gadgets.paillier;

import java.math.BigInteger;

import circuit.auxiliary.LongElement;
import circuit.operations.Gadget;
import circuit.structure.Wire;
import circuit.structure.WireArray;
import examples.gadgets.math.LongIntegerModGadget;

/***
 * Gadget which perform modular exponentiation in paillier
 * a**b [paillierModulus]
 */
public class ModPowPaillier extends Gadget {
	private Wire[] a;
	private int b;
	private Wire[] result;
	
	private LongElement paillierModulus;
	private int paillierKeyBitLength;
	public ModPowPaillier(LongElement paillierModulus, Wire[] a, int b, int paillierKeyBitLength, String... desc) {
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
		LongElement res = new LongElement(
			new BigInteger[] { BigInteger.ONE });
		//Naive approach
		for(int i = 0; i < this.b; i++){
			res = res.mul(msgA);
			res = new LongIntegerModGadget(res, this.paillierModulus, this.paillierKeyBitLength, true).getRemainder();
		}
		
		this.result = res.getBits(-1).packBitsIntoWords(8);
	}
	
	@Override
	public Wire[] getOutputWires() {
		return this.result;
	}
}
