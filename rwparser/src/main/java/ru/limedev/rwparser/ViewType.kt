package ru.limedev.rwparser

enum class ViewType(val rotationX: Int, val rotationY: Int, val rotationZ: Int) {

    /**
     * Specifies the initial rotation of the object so that the view is directed towards its floor.
     */
    MODEL_FLOOR(180, 0, 0),

    /**
     * Specifies the initial rotation of the object so that the view is directed towards its roof.
     */
    MODEL_ROOF(0, 0, 0),

    /**
     * Specifies the initial rotation of the object so that the view is directed towards its
     * front side.
     */
    MODEL_FRONT_SIDE(270, 0, 0),

    /**
     * Specifies the initial rotation of the object so that the view is directed towards its
     * back end side.
     */
    MODEL_BACK_END_SIDE(270, 180, 0)
}