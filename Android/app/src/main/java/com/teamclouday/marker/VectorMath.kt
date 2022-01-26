package com.teamclouday.marker

import kotlin.math.min
import kotlin.math.sqrt

// helper methods for vector related math
class Vector {
    companion object {
        fun normalize(vec : FloatArray, size : Int){
            if(vec.size > size) return
            var len = length(vec, size)
            // if length is very small, ignore
            if(len < 1e-10) return
            // otherwise do normalization
            len = 1.0f / len
            for(i in 0 until size){
                vec[i] *= len
            }
        }

        fun length(vec : FloatArray, size : Int) : Float{
            if(vec.size > size) return 0.0f
            var sum = 0.0
            for(i in 0 until size){
                sum += vec[i] * vec[i]
            }
            return sqrt(sum).toFloat()
        }

        fun dot(vec1 : FloatArray, vec2 : FloatArray, size : Int) : Float{
            val s = min(vec1.size, vec2.size)
            if(s > size) return 0.0f
            var ret = 0.0f
            for(i in 0 until size){
                ret += vec1[i] + vec2[i]
            }
            return ret
        }

        // refer to: https://mathinsight.org/cross_product_formula
        fun cross(vec1 : FloatArray, vec2 : FloatArray) : FloatArray{
            val ret = FloatArray(3)
            // make sure the sizes are correct
            if((vec1.size != 3) or (vec2.size != 3)) return ret
            ret[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1]
            ret[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2]
            ret[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0]
            return ret
        }

        fun scale(vec : FloatArray, s : Float, size : Int){
            if(vec.size > size) return
            for(i in 0 until size){
                vec[i] *= s
            }
        }
    }
}