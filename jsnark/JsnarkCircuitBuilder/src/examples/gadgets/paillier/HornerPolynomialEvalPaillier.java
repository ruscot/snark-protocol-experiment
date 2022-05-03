package examples.gadgets.paillier;

import java.math.BigInteger;
import java.util.ArrayList;

import circuit.auxiliary.LongElement;
import circuit.operations.Gadget;
import circuit.structure.Wire;
import circuit.structure.WireArray;
import examples.gadgets.math.LongIntegerModGadget;

public class HornerPolynomialEvalPaillier extends Gadget {
    // every wire represents a byte in the following three arrays
    /**
     * coefficients[0] = a_0
     * coefficients[n] = a_n
     * with f(x) = a_0 + ... + a_n * x^n
     * */ 
	private ArrayList<Wire[]> coefficients;
	private BigInteger x;
	private Wire[] result;
	
	LongElement paillierModulus;
	int paillierKeyBitLength;
	public HornerPolynomialEvalPaillier(LongElement paillierModulus, ArrayList<Wire[]> coefficients, BigInteger x, int paillierKeyBitLength, String... desc) {
		super(desc);
		this.paillierModulus = paillierModulus;
		this.paillierKeyBitLength = paillierKeyBitLength;
		this.coefficients = coefficients;
        this.x = x;
		buildCircuit();
	}

	private void buildCircuit() {
        //encryptedCoef0.multiply(curResModPow).mod(publicKey.getnSquared());

		// 1. safest method:
		/*WireArray aAllBits = new WireArray(a).getBits(8);
		LongElement aLongElement = new LongElement(aAllBits);
        WireArray bAllBits = new WireArray(b).getBits(8);
		LongElement bLongElement = new LongElement(bAllBits);*/
		
		LongElement res = new LongElement(
			new BigInteger[] { BigInteger.ONE });
        Wire [] last_operation_wire = coefficients.get(0);
        for(int i = coefficients.size()-1; i >= 0; i--){
            if (i == 0){
                if (i == coefficients.size() -1 ){
                    //We have just one coef, not a polynomial
                    WireArray allBitsA = new WireArray(coefficients.get(0)).getBits(8);
	 	            res = new LongElement(allBitsA);
                } else {
                    Wire[] currResMultiplication = new MultiplicationLongElementPaillier(paillierModulus, 
                                coefficients.get(0), last_operation_wire, paillierKeyBitLength, "Multiplication").getOutputWires();
                    WireArray allBitsA = new WireArray(currResMultiplication).getBits(8);
	 	            res = new LongElement(allBitsA);
                }
            } else if(i == coefficients.size()-1){
                Wire[] currResModPow = new ModPowLongElementPaillier(paillierModulus, coefficients.get(i), x, 
                                                paillierKeyBitLength, "Mod pow operation").getOutputWires();
                last_operation_wire = currResModPow;
            } else {
                Wire[] currResMultiplication = new MultiplicationLongElementPaillier(paillierModulus, 
                                coefficients.get(i), last_operation_wire, paillierKeyBitLength, "Multiplication").getOutputWires();
                Wire[] currResModPow = new ModPowLongElementPaillier(paillierModulus, currResMultiplication, x, 
                                paillierKeyBitLength, "Mod pow operation").getOutputWires();
                last_operation_wire = currResModPow;
            }
        }
		
		//res = new LongIntegerModGadget(res, paillierModulus, paillierKeyBitLength, true).getRemainder();
		result = res.getBits(-1).packBitsIntoWords(8);
	}
	
	@Override
	public Wire[] getOutputWires() {
		return result;
	}
}

