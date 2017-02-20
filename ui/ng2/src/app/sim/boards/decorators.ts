import { AbstractBoard } from './AbstractBoard';


// http://stackoverflow.com/q/29775830/

export function ui_catcher(
    target: Object,
    propertyKey: string,
    descriptor: TypedPropertyDescriptor<any>): TypedPropertyDescriptor<any> {

  const originalMethod = descriptor.value; // save a reference to the original method

  // NOTE: Do not use arrow syntax here. Use a function expression in
  // order to use the correct value of `this` in this method (see notes below)
  descriptor.value = function(...args: any[]): Object {
    try {
      // run and return the result of the original method
      return originalMethod.apply(this, args);
    } catch (err) {
      const this_: AbstractBoard = this;
      this_._controller.errorOccured(err);
    }
  };

  return descriptor;
}
