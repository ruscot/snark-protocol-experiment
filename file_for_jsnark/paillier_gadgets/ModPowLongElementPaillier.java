package examples.gadgets.paillier;

import java.math.BigInteger;

import circuit.auxiliary.LongElement;
import circuit.operations.Gadget;
import circuit.structure.Wire;
import circuit.structure.WireArray;
import examples.gadgets.math.LongIntegerModGadget;

/***
 * Gadget which perform modular exponentiation in paillier with b a big int 
 * a**b [paillierModulus]
 */
public class ModPowLongElementPaillier extends Gadget {
	private Wire[] a;
	private BigInteger b;
	private Wire[] result;
	
	private LongElement paillierModulus;
	private int paillierKeyBitLength;

	public ModPowLongElementPaillier(LongElement paillierModulus, Wire[] a, BigInteger b, int paillierKeyBitLength, String... desc) {
		super(desc);
		this.paillierModulus = paillierModulus;
		this.paillierKeyBitLength = paillierKeyBitLength;
		this.a = a;
        this.b = b;
		buildCircuit();
	}

	private void buildCircuit() {
		WireArray allBitsA = new WireArray(this.a).getBits(8);
	 	LongElement msgA = new LongElement(allBitsA);
		LongElement res = new LongElement(
					new BigInteger[] { BigInteger.ONE });
		
		
		/*System.out.println("");
		System.out.println("//----------------------------------------------------------------------------");
		System.out.println("");
		System.out.println("In build circuit try to manipulate LongElement");
		System.out.println("res value " + res.getSize());
		WireArray resAsArraay = res.getBits();
		System.out.println("Wire array of res " + resAsArraay.toString());
		System.out.println("In build circuit finish our manipulation");
		System.out.println("");
		System.out.println("//----------------------------------------------------------------------------");
		System.out.println("");
		System.out.println("");
		System.out.println("");
		exit(0) ;*/
		
		//fast exponentiation
		int bitLengthB = b.bitLength();
		System.out.println("Bit length b " + b.bitLength());
		for(int j = 0; j < 25; j++){
			/*if(bitLengthB == j){
				break;
			}*/
			System.out.println("In fast expo iteration " + j);
			//if(b.getLowestSetBit() == 0){ //b is odd
				res = res.mul(msgA);
				res = new LongIntegerModGadget(res, this.paillierModulus, 
						this.paillierKeyBitLength, true).getRemainder();
			//} 
			this.b = this.b.shiftRight(1);
			msgA = msgA.mul(msgA);
			msgA = new LongIntegerModGadget(msgA, this.paillierModulus, 
					this.paillierKeyBitLength, true).getRemainder();
		}
		
		/*while(this.b.compareTo(BigInteger.ONE) == 1 || this.b.equals(BigInteger.ONE)){ //equal b > 1
			System.out.println("In fast expo while " + i);
			if(!this.b.mod(BigInteger.TWO).equals(BigInteger.ZERO)){ //b is odd
				res = res.mul(msgA);
				res = new LongIntegerModGadget(res, this.paillierModulus, this.paillierKeyBitLength, true).getRemainder();
				//this.b = this.b.subtract(BigInteger.ONE);
			} 
			this.b = this.b.shiftRight(1);
			msgA = msgA.mul(msgA);
			msgA = new LongIntegerModGadget(msgA, this.paillierModulus, this.paillierKeyBitLength, true).getRemainder();
			i ++;
		}
		System.out.println("After the fast exponentiation");*/

		this.result = res.getBits(-1).packBitsIntoWords(8);
	}

	@Override
	public Wire[] getOutputWires() {
		return this.result;
	}
}
